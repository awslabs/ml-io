/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 *      http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#include "mlio/parallel_data_reader.h"

#include <cstddef>
#include <tuple>
#include <utility>
#include <vector>

#include <tbb/tbb.h>

#include "mlio/data_reader.h"
#include "mlio/detail/thread.h"
#include "mlio/example.h"
#include "mlio/instance_batch.h"
#include "mlio/instance_batch_reader.h"
#include "mlio/instance_readers/instance_reader.h"
#include "mlio/record_readers/record_reader.h"

using mlio::detail::Instance_batch_reader;

namespace mlio {
inline namespace abi_v1 {

// Used as a message in the TBB flow graph.
struct Batch_msg {
    std::shared_ptr<Instance_batch> batch{};
};

// Used as a message in the TBB flow graph.
struct Example_msg {
    std::size_t idx{};
    Intrusive_ptr<Example> example{};
};

// Holds the internal TBB flow graph objects.
struct Parallel_data_reader::Graph_data {
    tbb::task_group_context ctx{};
    tbb::flow::graph obj{ctx};
    tbb::flow::source_node<Batch_msg> *src_node{};
    std::vector<std::unique_ptr<tbb::flow::graph_node>> nodes{};
};

Parallel_data_reader::~Parallel_data_reader() = default;

std::size_t Parallel_data_reader::num_bytes_read() const noexcept
{
    return num_bytes_read_;
}

Parallel_data_reader::Parallel_data_reader(Data_reader_params &&params)
    : Data_reader_base{std::move(params)}, graph_{std::make_unique<Graph_data>()}
{
    reader_ = detail::make_instance_reader(this->params(), [this](const Data_store &store) {
        return make_record_reader(store);
    });

    batch_reader_ = std::make_unique<Instance_batch_reader>(this->params(), *reader_);
}

void Parallel_data_reader::stop()
{
    if (state_ == Run_state::not_started) {
        return;
    }

    read_queue_.clear();

    {
        std::unique_lock<std::mutex> queue_lock{queue_mutex_};

        graph_->ctx.cancel_group_execution();

        fill_queue_.clear();
    }

    fill_condition_.notify_one();

    thread_.join();
}

Intrusive_ptr<Example> Parallel_data_reader::read_example_core()
{
    ensure_schema_inferred();

    //               ┌───< read_example_core() <───┐
    //               │                             │
    //               │                             │
    //           Fill Queue                    Read Queue
    //               │                             │
    //               │                             │
    //               └─────> Background Thr. >─────┘
    //
    //
    // The read_example_core function pops the items available in the
    // read queue and swaps the read and fill queues once the read queue
    // is empty.
    //
    // The background thread continuously pushes items returned from the
    // flow graph into the fill queue.
    if (read_queue_.empty()) {
        ensure_pipeline_running();

        {
            std::unique_lock<std::mutex> queue_lock{queue_mutex_};

            read_condition_.wait(queue_lock, [this] {
                return state_ != Run_state::running || !fill_queue_.empty();
            });

            if (state_ == Run_state::faulted) {
                std::rethrow_exception(exception_ptr_);
            }

            using std::swap;

            swap(read_queue_, fill_queue_);
        }

        fill_condition_.notify_one();
    }

    if (read_queue_.empty()) {
        return {};
    }

    Intrusive_ptr<Example> example = std::move(read_queue_.front());

    read_queue_.pop_front();

    return example;
}

void Parallel_data_reader::ensure_pipeline_running()
{
    if (state_ != Run_state::not_started) {
        return;
    }

    state_ = Run_state::running;

    thread_ = detail::start_thread(&Parallel_data_reader::run_pipeline, this);
}

void Parallel_data_reader::run_pipeline()
{
    if (graph_->src_node == nullptr) {
        init_graph();
    }

    graph_->src_node->activate();

    try {
        graph_->obj.wait_for_all();
    }
    catch (const std::exception &) {
        exception_ptr_ = std::current_exception();
    }

    {
        std::unique_lock<std::mutex> queue_lock{queue_mutex_};

        if (exception_ptr_) {
            state_ = Run_state::faulted;
        }
        else {
            state_ = Run_state::stopped;
        }
    }

    read_condition_.notify_one();
}

void Parallel_data_reader::init_graph()
{
    namespace flw = tbb::flow;

    std::size_t num_prefetched_examples = params().num_prefetched_examples;
    if (num_prefetched_examples == 0) {
        // Defaults to the number of processor cores.
        num_prefetched_examples =
            static_cast<std::size_t>(tbb::task_scheduler_init::default_num_threads());
    }

    std::size_t num_parallel_reads = params().num_parallel_reads;
    if (num_parallel_reads == 0 || num_parallel_reads > num_prefetched_examples) {
        num_parallel_reads = num_prefetched_examples;
    }

    flw::graph &g = graph_->obj;

    // Source
    auto src_node = std::make_unique<flw::source_node<Batch_msg>>(
        g,
        [this](auto &msg) {
            std::optional<Instance_batch> batch = batch_reader_->read_instance_batch();
            if (batch == std::nullopt) {
                return false;
            }

            msg = Batch_msg{std::make_shared<Instance_batch>(std::move(*batch))};

            return true;
        },
        false);

    // Limiter
    auto limit_node = std::make_unique<flw::limiter_node<Batch_msg>>(g, num_parallel_reads);

    // Decode
    auto decode_node =
        std::make_unique<flw::multifunction_node<Batch_msg, std::tuple<Example_msg>>>(
            g, flw::unlimited, [this](const auto &msg, auto &ports) {
                // We send a message to the next node even if the decode
                // function fails. This is needed to have correct
                // sequential ordering of other batches.
                Example_msg out{msg.batch->index(), this->decode(*msg.batch)};

                if (out.example != nullptr) {
                    num_bytes_read_.fetch_add(msg.batch->size_bytes());
                }

                std::get<0>(ports).try_put(std::move(out));
            });

    // Order
    auto order_node = std::make_unique<flw::sequencer_node<Example_msg>>(g, [](const auto &msg) {
        return msg.idx;
    });

    // Queue
    auto queue_node = std::make_unique<flw::function_node<Example_msg, flw::continue_msg>>(
        g, flw::serial, [this, num_prefetched_examples](const auto &msg) {
            // If the decode function has failed discard the message.
            if (msg.example == nullptr) {
                return;
            }

            {
                std::unique_lock<std::mutex> queue_lock{queue_mutex_};

                fill_condition_.wait(queue_lock, [this, num_prefetched_examples] {
                    return fill_queue_.size() < num_prefetched_examples;
                });

                if (graph_->ctx.is_group_execution_cancelled()) {
                    return;
                }

                fill_queue_.push_back(msg.example);
            }

            read_condition_.notify_one();
        });

    flw::make_edge(*src_node, *limit_node);
    flw::make_edge(*limit_node, *decode_node);
    flw::make_edge(flw::output_port<0>(*decode_node), *order_node);
    flw::make_edge(*order_node, *queue_node);
    flw::make_edge(*queue_node, limit_node->decrement);

    graph_->src_node = src_node.get();

    graph_->nodes.emplace_back(std::move(src_node));
    graph_->nodes.emplace_back(std::move(limit_node));
    graph_->nodes.emplace_back(std::move(decode_node));
    graph_->nodes.emplace_back(std::move(order_node));
    graph_->nodes.emplace_back(std::move(queue_node));
}

void Parallel_data_reader::ensure_schema_inferred()
{
    if (schema_) {
        return;
    }

    schema_ = infer_schema(reader_->peek_instance());
}

Intrusive_ptr<const Schema> Parallel_data_reader::read_schema()
{
    ensure_schema_inferred();

    return schema_;
}

void Parallel_data_reader::reset() noexcept
{
    stop();

    state_ = Run_state::not_started;

    batch_reader_->reset();

    graph_->ctx.reset();

    graph_->obj.reset();

    exception_ptr_ = nullptr;

    num_bytes_read_ = 0;

    Data_reader_base::reset();
}

}  // namespace abi_v1
}  // namespace mlio
