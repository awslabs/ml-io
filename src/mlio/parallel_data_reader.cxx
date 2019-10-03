/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#include "mlio/default_instance_reader.h"
#include "mlio/detail/thread.h"
#include "mlio/example.h"
#include "mlio/instance_batch.h"
#include "mlio/instance_batch_reader.h"
#include "mlio/instance_reader.h"
#include "mlio/optional.h"
#include "mlio/shuffled_instance_reader.h"

using mlio::detail::default_instance_reader;
using mlio::detail::instance_batch_reader;
using mlio::detail::shuffled_instance_reader;

namespace mlio {
inline namespace v1 {

// Used as a message in the TBB flow graph.
struct batch_msg {
    std::shared_ptr<instance_batch> batch{};
};

// Used as a message in the TBB flow graph.
struct example_msg {
    std::size_t idx{};
    intrusive_ptr<example> exm{};
};

// Holds the TBB flow graph objects.
struct parallel_data_reader::graph_data {
    tbb::task_group_context ctx{};
    tbb::flow::graph obj{ctx};
    tbb::flow::source_node<batch_msg> *src_node{};
    std::vector<std::unique_ptr<tbb::flow::graph_node>> nodes{};
};

parallel_data_reader::
parallel_data_reader(data_reader_params &&prm)
    : data_reader_base{std::move(prm)}, graph_{std::make_unique<graph_data>()}
{
    data_reader_params const &prms = params();

    reader_ = std::make_unique<default_instance_reader>(prms,
        [this](data_store const &ds)
        {
            return make_record_reader(ds);
        });

    if (prms.shuffle_instances) {
        reader_ = std::make_unique<shuffled_instance_reader>(
            prms,
            std::move(reader_));
    }

    batch_reader_ = std::make_unique<instance_batch_reader>(prms, *reader_);
}

parallel_data_reader::~parallel_data_reader() = default;

intrusive_ptr<example>
parallel_data_reader::
read_example_core()
{
    ensure_schema_inferred();

    //                   ┌───< read_example_core() <───┐
    //                   │                             │
    //                   │                             │
    //               Fill Queue                    Read Queue
    //                   │                             │
    //                   │                             │
    //                   └─────> Background Thr. >─────┘
    //
    //
    // The read_example_core function pops the items available in the
    // read queue and swaps the read and fill queues once the read queue
    // is empty 
    //
    // The background thread continuously pushes items returned from the
    // flow graph into the fill queue.
    if (read_queue_.empty()) {
        ensure_pipeline_running();

        {
            std::unique_lock<std::mutex> queue_lock{queue_mutex_};

            read_cond_.wait(queue_lock, [this]
            {
                return state_ != state::running || !fill_queue_.empty();
            });

            if (state_ == state::faulted) {
                std::rethrow_exception(exception_ptr_);
            }

            std::swap(read_queue_, fill_queue_);
        }

        fill_cond_.notify_one();
    }

    if (read_queue_.empty()) {
        return {};
    }

    intrusive_ptr<example> exm = std::move(read_queue_.front());

    read_queue_.pop_front();

    return exm;
}

void
parallel_data_reader::
ensure_pipeline_running()
{
    if (state_ != state::not_started) {
        return;
    }

    state_ = state::running;

    thrd_ = detail::start_thread(&parallel_data_reader::run_pipeline, this);
}

void
parallel_data_reader::
run_pipeline()
{
    if (graph_->src_node == nullptr) {
        init_graph();
    }

    graph_->src_node->activate();

    try {
        graph_->obj.wait_for_all();
    }
    catch (std::exception const &) {
        exception_ptr_ = std::current_exception();
    }

    {
        std::unique_lock<std::mutex> queue_lock{queue_mutex_};

        if (exception_ptr_) {
            state_ = state::faulted;
        } else {
            state_ = state::stopped;
        }
    }

    read_cond_.notify_one();
}

void
parallel_data_reader::
init_graph()
{
    namespace flw = tbb::flow;

    std::size_t num_prefetched_batches = params().num_prefetched_batches;
    if (num_prefetched_batches == 0) {
        // Defaults to the number of processor cores.
        num_prefetched_batches =
            static_cast<std::size_t>(tbb::task_scheduler_init::default_num_threads());
    }

    std::size_t num_parallel_reads = params().num_parallel_reads;
    if (num_parallel_reads == 0 || num_parallel_reads > num_prefetched_batches) {
        num_parallel_reads = num_prefetched_batches;
    }

    flw::graph &g = graph_->obj;

    // Source
    auto src_node = std::make_unique<flw::source_node<batch_msg>>(g,
    [this](auto &msg)
    {
        stdx::optional<instance_batch> btch = batch_reader_->read_instance_batch();
        if (btch == stdx::nullopt) {
            return false;
        }

        msg = batch_msg{std::make_shared<instance_batch>(std::move(*btch))};

        return true;
    }, false);

    // Limiter
    auto limit_node = std::make_unique<
        flw::limiter_node<batch_msg>>(g, num_parallel_reads);

    // Decode
    auto decode_node = std::make_unique<
        flw::multifunction_node<batch_msg, std::tuple<example_msg>>>(g, flw::unlimited,
    [this](auto const &msg, auto &ports)
    {
        // We send a message to the next node even if the decode()
        // function fails. This is needed to have correct sequential
        // ordering of other batches.
        example_msg out{msg.batch->index(), this->decode(*msg.batch)};

        std::get<0>(ports).try_put(std::move(out));
    });

    // Order
    auto order_node = std::make_unique<flw::sequencer_node<example_msg>>(g,
    [](auto const &msg)
    {
        return msg.idx;
    });

    // Queue
    auto queue_node = std::make_unique<
        flw::function_node<example_msg, flw::continue_msg>>(g, flw::serial,
    [this, num_prefetched_batches](auto const &msg)
    {
        // If the decode() function has failed simply discard the
        // message.
        if (msg.exm == nullptr) {
            return;
        }

        {
            std::unique_lock<std::mutex> queue_lock{queue_mutex_};

            fill_cond_.wait(queue_lock, [this, num_prefetched_batches]
            {
                return fill_queue_.size() < num_prefetched_batches;
            });

            if (graph_->ctx.is_group_execution_cancelled()) {
                return;
            }

            fill_queue_.push_back(msg.exm);
        }

        read_cond_.notify_one();
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

void
parallel_data_reader::
ensure_schema_inferred()
{
    if (schema_inferred_) {
        return;
    }

    stdx::optional<instance> const &ins = reader_->peek_instance();
    if (ins == stdx::nullopt) {
        return;
    }

    infer_schema(*ins);

    schema_inferred_ = true;
}

void
parallel_data_reader::
reset() noexcept
{
    stop();

    state_ = state::not_started;

    reader_->reset();

    graph_->ctx.reset();

    graph_->obj.reset();

    exception_ptr_ = nullptr;
}

void
parallel_data_reader::
stop()
{
    if (state_ == state::not_started) {
        return;
    }

    read_queue_.clear();

    {
        std::unique_lock<std::mutex> queue_lock{queue_mutex_};

        graph_->ctx.cancel_group_execution();

        fill_queue_.clear();
    }

    fill_cond_.notify_one();

    thrd_.join();
}

}  // namespace v1
}  // namespace mlio
