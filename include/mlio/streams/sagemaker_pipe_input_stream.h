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

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>

#include "mlio/config.h"
#include "mlio/detail/file_descriptor.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream_base.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

struct Sagemaker_pipe_input_stream_access;

}  // namespace detail

/// @addtogroup streams Streams
/// @{

class MLIO_API Sagemaker_pipe_input_stream final : public Input_stream_base {
    friend struct detail::Sagemaker_pipe_input_stream_access;

public:
    Sagemaker_pipe_input_stream(const Sagemaker_pipe_input_stream &) = delete;

    Sagemaker_pipe_input_stream &operator=(const Sagemaker_pipe_input_stream &) = delete;

    Sagemaker_pipe_input_stream(Sagemaker_pipe_input_stream &&) = delete;

    Sagemaker_pipe_input_stream &operator=(Sagemaker_pipe_input_stream &&) = delete;

    ~Sagemaker_pipe_input_stream() final;

    using Input_stream_base::read;

    std::size_t read(Mutable_memory_span destination) final;

    void close() noexcept final;

    bool closed() const noexcept final
    {
        return !fifo_fd_.is_open();
    }

private:
    explicit Sagemaker_pipe_input_stream(std::string &&path,
                                         std::chrono::seconds timeout,
                                         std::optional<std::size_t> fifo_id);

    MLIO_HIDDEN
    void open_fifo();

    MLIO_HIDDEN
    static void sleep() noexcept;

    MLIO_HIDDEN
    void wait_for_data();

    MLIO_HIDDEN
    void check_if_closed() const;

    std::string path_;
    std::ptrdiff_t fifo_id_ = -1;
    detail::File_descriptor fifo_fd_{};
    std::chrono::seconds timeout_;
};

inline constexpr std::chrono::seconds sagemaker_pipe_default_timeout{60};

MLIO_API
Intrusive_ptr<Sagemaker_pipe_input_stream>
make_sagemaker_pipe_input_stream(std::string path,
                                 std::chrono::seconds timeout = sagemaker_pipe_default_timeout,
                                 std::optional<std::size_t> fifo_id = {});

/// @}

}  // namespace abi_v1
}  // namespace mlio
