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

#pragma once

#include <atomic>
#include <condition_variable>
#include <deque>
#include <exception>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>

#include "mlio/config.h"
#include "mlio/data_reader.h"
#include "mlio/data_reader_base.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents an abstract helper base class for data readers
/// that support multi-threading.
class MLIO_API Parallel_data_reader : public Data_reader_base {
public:
    Parallel_data_reader(const Parallel_data_reader &) = delete;

    Parallel_data_reader &operator=(const Parallel_data_reader &) = delete;

    Parallel_data_reader(Parallel_data_reader &&) = delete;

    Parallel_data_reader &operator=(Parallel_data_reader &&) = delete;

    ~Parallel_data_reader() override;

    Intrusive_ptr<const Schema> read_schema() final;

    void reset() noexcept override;

    std::size_t num_bytes_read() const noexcept final;

protected:
    explicit Parallel_data_reader(Data_reader_params &&params);

    /// Stops the background threads. This function must be called in
    /// the destructor of the derived class to ensure that all
    /// resources are properly disposed.
    void stop();

    Intrusive_ptr<const Schema> schema() const noexcept
    {
        return schema_;
    }

private:
    enum class Run_state { not_started, running, stopped, faulted };

    struct Graph_data;

    /// When implemented in a derived class, constructs a @ref
    /// Record_reader from the specified data store.
    virtual Intrusive_ptr<Record_reader> make_record_reader(const Data_store &store) = 0;

    MLIO_HIDDEN
    Intrusive_ptr<Example> read_example_core() final;

    MLIO_HIDDEN
    void ensure_pipeline_running();

    MLIO_HIDDEN
    void run_pipeline();

    MLIO_HIDDEN
    void init_graph();

    MLIO_HIDDEN
    void ensure_schema_inferred();

    /// When implemented in a derived class, infers the Schema of the
    /// dataset from the specified data Instance.
    ///
    /// @remark
    ///     This function will be called once throughout the lifetime
    ///     of the data reader before any calls to the @ref decode()
    ///     function.
    virtual Intrusive_ptr<const Schema> infer_schema(const std::optional<Instance> &instance) = 0;

    /// When implemented in a derived class, converts the specified
    /// @ref Instance_batch into an @ref Example.
    virtual Intrusive_ptr<Example> decode(const Instance_batch &batch) const = 0;

    Data_reader_params params_;
    std::unique_ptr<detail::Instance_reader> reader_;
    std::unique_ptr<detail::Instance_batch_reader> batch_reader_;
    Run_state state_{};
    std::unique_ptr<Graph_data> graph_;
    std::thread thread_{};
    std::deque<Intrusive_ptr<Example>> fill_queue_{};
    std::deque<Intrusive_ptr<Example>> read_queue_{};
    std::mutex queue_mutex_;
    std::condition_variable fill_condition_{};
    std::condition_variable read_condition_{};
    std::exception_ptr exception_ptr_{};
    std::atomic_size_t num_bytes_read_{};
    Intrusive_ptr<const Schema> schema_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
