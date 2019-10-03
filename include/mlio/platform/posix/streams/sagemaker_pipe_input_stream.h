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

// IWYU pragma: private, include "mlio/streams/sagemaker_pipe_input_stream.h"

#include <cstddef>
#include <string>

#include "mlio/config.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/platform/posix/detail/file_descriptor.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream_base.h"

namespace mlio {
inline namespace v1 {
namespace detail {

struct sagemaker_pipe_input_stream_access;

}  // namespace detail

/// @addtogroup streams Streams
/// @{

class MLIO_API sagemaker_pipe_input_stream final : public input_stream_base {
    friend struct detail::sagemaker_pipe_input_stream_access;

private:
    explicit
    sagemaker_pipe_input_stream(std::string &&pathname);

public:
    sagemaker_pipe_input_stream(sagemaker_pipe_input_stream const &) = delete;

    sagemaker_pipe_input_stream(sagemaker_pipe_input_stream &&) = delete;

   ~sagemaker_pipe_input_stream() final;

public:
    sagemaker_pipe_input_stream &
    operator=(sagemaker_pipe_input_stream const &) = delete;

    sagemaker_pipe_input_stream &
    operator=(sagemaker_pipe_input_stream &&) = delete;

public:
    using input_stream_base::read;

    std::size_t
    read(mutable_memory_span dest) final;

    void
    close() noexcept final;

private:
    MLIO_HIDDEN
    void
    open_fifo();

    MLIO_HIDDEN
    static void
    sleep() noexcept;

    MLIO_HIDDEN
    void
    wait_for_data();

    MLIO_HIDDEN
    void
    check_if_closed() const;

public:
    bool
    closed() const noexcept final
    {
        return !fifo_fd_.is_open();
    }

private:
    std::string pathname_;
    std::ptrdiff_t fifo_id_ = -1;
    detail::file_descriptor fifo_fd_{};
};

MLIO_API
intrusive_ptr<sagemaker_pipe_input_stream>
make_sagemaker_pipe_input_stream(std::string pathname);

/// @}

}  // namespace v1
}  // namespace mlio
