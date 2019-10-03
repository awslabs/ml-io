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

#pragma once

#include <condition_variable>
#include <deque>
#include <exception>
#include <memory>
#include <mutex>
#include <thread>

#include "mlio/config.h"
#include "mlio/data_reader.h"
#include "mlio/data_reader_base.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents an abstract helper base class for data readers
/// that support multi-threading.
class MLIO_API parallel_data_reader : public data_reader_base {
    enum class state {
        not_started,
        running,
        stopped,
        faulted
    };

    struct graph_data;

protected:
    explicit
    parallel_data_reader(data_reader_params &&prm);

public:
    parallel_data_reader(parallel_data_reader const &) = delete;

    parallel_data_reader(parallel_data_reader &&) = delete;

   ~parallel_data_reader() override;

public:
    parallel_data_reader &
    operator=(parallel_data_reader const &) = delete;

    parallel_data_reader &
    operator=(parallel_data_reader &&) = delete;

private:
    /// When implemented in a derived class, constructs a @ref
    /// record_reader from the specified data store.
    virtual intrusive_ptr<record_reader>
    make_record_reader(data_store const &ds) = 0;

    MLIO_HIDDEN
    intrusive_ptr<example>
    read_example_core() final;

    MLIO_HIDDEN
    void
    ensure_pipeline_running();

    MLIO_HIDDEN
    void
    run_pipeline();

    MLIO_HIDDEN
    void
    init_graph();

    MLIO_HIDDEN
    void
    ensure_schema_inferred();

    /// When implemented in a derived class, infers the schema of the
    /// dataset from the specified data instance.
    ///
    /// @remark
    ///     This function will be called once throughout the lifetime
    ///     of the data reader before any calls to the @ref decode()
    ///     function.
    virtual void
    infer_schema(instance const &ins) = 0;

    /// When implemented in a derived class, converts the specified
    /// @ref instance_batch into an @ref example.
    virtual intrusive_ptr<example>
    decode(instance_batch const &batch) const = 0;

public:
    void
    reset() noexcept final;

protected:
    /// Stops the background threads. This function must be called in
    /// the destructor of the derived class to ensure that all
    /// resources are properly disposed.
    void
    stop();

private:
    data_reader_params params_;
    std::unique_ptr<detail::instance_reader> reader_;
    std::unique_ptr<detail::instance_batch_reader> batch_reader_;
    state state_{};
    std::unique_ptr<graph_data> graph_;
    std::thread thrd_{};
    std::deque<intrusive_ptr<example>> fill_queue_{};
    std::deque<intrusive_ptr<example>> read_queue_{};
    std::mutex queue_mutex_;
    std::condition_variable fill_cond_{};
    std::condition_variable read_cond_{};
    std::exception_ptr exception_ptr_{};
    bool schema_inferred_{};
};

/// @}

}  // namespace v1
}  // namespace mlio
