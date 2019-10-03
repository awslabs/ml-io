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

#include "mlio/streams/sagemaker_pipe_input_stream.h"  // IWYU pragma: associated

#include <atomic>
#include <cerrno>
#include <chrono>
#include <thread>
#include <unordered_map>
#include <utility>
#include <system_error>

#include <fcntl.h>
#include <sys/select.h>
#include <time.h>  // NOLINT(modernize-deprecated-headers)
#include <unistd.h>

#include <fmt/format.h>

#include "mlio/detail/error.h"
#include "mlio/detail/pathname.h"
#include "mlio/logger.h"
#include "mlio/platform/posix/detail/system_call.h"
#include "mlio/streams/stream_error.h"

using mlio::detail::current_error_code;
using mlio::detail::sagemaker_pipe_input_stream_access;

namespace mlio {
inline namespace v1 {
namespace detail {

struct sagemaker_pipe_input_stream_access {
    static inline auto
    make(std::string &&pathname)
    {
        auto *ptr = new sagemaker_pipe_input_stream{std::move(pathname)};

        auto strm = wrap_intrusive(ptr);

        strm->open_fifo();

        return strm;
    }
};

namespace {

// For each SageMaker pipe channel that is opened by this process, this
// container holds the pathname-to-FIFO mapping. If a new
// sagemaker_pipe_input_stream is instantiated, it replaces the FIFO ID
// with -1 indicating that it acquired that pipe channel. Right before
// getting destructed, the stream increments the original FIFO ID and
// puts it back to the container.
std::unordered_map<std::string, std::atomic_ptrdiff_t> fifo_ids_{};

}  // namespace
}  // namespace detail

intrusive_ptr<sagemaker_pipe_input_stream>
make_sagemaker_pipe_input_stream(std::string pathname)
{
    return sagemaker_pipe_input_stream_access::make(std::move(pathname));
}

sagemaker_pipe_input_stream::
sagemaker_pipe_input_stream(std::string &&pathname)
    : pathname_{std::move(pathname)}
{
    detail::validate_file_pathname(pathname_);

    fifo_id_ = detail::fifo_ids_[pathname_].exchange(-1);
    if (fifo_id_ == -1) {
        auto err = std::make_error_code(std::errc::permission_denied);
        throw std::system_error{err,
            "The SageMaker pipe channel is already open."};
    }
}

sagemaker_pipe_input_stream::~sagemaker_pipe_input_stream()
{
    close();
}

void
sagemaker_pipe_input_stream::
open_fifo()
{
    std::string fifo_name = fmt::format("{0}_{1}", pathname_, fifo_id_);

    constexpr int max_num_attempts = 3;

    // Although the default behavior of pipe mode is to create the next
    // FIFO in advance there is no guarantee for that. In order to make
    // sure that we do not fail if it has not been created yet we use a
    // basic retry logic.
    int attempt_count = 0;
    while (true) {
        // We open the read end of the FIFO in non-blocking mode to make
        // sure that we do not block indefinitely in case SageMaker
        // fails to open the write end.
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE >= 200809L
        fifo_fd_ = ::open(fifo_name.c_str(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
#else
        fifo_fd_ = ::open(fifo_name.c_str(), O_RDONLY | O_NONBLOCK);
#endif

        if (fifo_fd_ != -1) {
            logger::debug(
                "FIFO {0:n} of the SageMaker pipe channel '{1}' is opened.",
                fifo_id_, pathname_);

            fifo_id_++;

            // We make sure that the write end is opened by waiting for
            // some data to be written into the FIFO buffer; otherwise
            // our first read attempt will have zero-bytes which will
            // wrongfully indicate EOF to the caller.
            wait_for_data();

            return;
        }

        if (++attempt_count == max_num_attempts) {
            break;
        }

        sleep();
    }

    std::error_code err = current_error_code();
    throw std::system_error{err, fmt::format(
        "FIFO {0:n} of the SageMaker pipe channel cannot be opened.",
        fifo_id_)};
}

void
sagemaker_pipe_input_stream::
sleep() noexcept
{
    constexpr std::chrono::seconds attempt_pause{1};

    ::timespec req{};
    req.tv_sec = attempt_pause.count();

    // We do not care about the signal interrupts here as we sleep for a
    // brief period of time and repeat the call in case the FIFO is not
    // there yet.
    ::nanosleep(&req, nullptr);
}

std::size_t
sagemaker_pipe_input_stream::
read(mutable_memory_span dest)
{
    check_if_closed();

    if (dest.empty()) {
        return 0;
    }

    int attempt_count = 0;
    while (true) {
        // We do not want the read operation to block indefinitely in
        // case SageMaker cannot deliver any data. However we also want
        // to avoid the cost of a wait operation. So we eagerly attempt
        // to read from the FIFO before checking if there is any data
        // available. As long as the write end behaves as expected this
        // call won't fail and it will spare us the costly system calls.
        ssize_t num_bytes_read = ::read(fifo_fd_.get(), dest.data(), dest.size());
        if (num_bytes_read == -1) {
            if (errno == EAGAIN) {
                attempt_count++;

                if (attempt_count == 1) {
                    // In case the caller consumes the stream extremely
                    // fast the write end might not be able to keep up
                    // with the pace. After our first failed read
                    // attempt instead of blocking right away we give
                    // the write end a bit breathing room. This trick
                    // slightly increases the throughput.
                    std::this_thread::yield();
                    continue;
                }

                if (attempt_count == 2) {
                    wait_for_data();
                    continue;
                }
            }

            std::error_code err = current_error_code();
            throw std::system_error{err, fmt::format(
                "FIFO {0:n} of the SageMaker pipe channel cannot be read.",
                fifo_id_)};
        }

        return static_cast<std::size_t>(num_bytes_read);
    }
}

void
sagemaker_pipe_input_stream::
wait_for_data()
{
    ::fd_set readfds{};
    // NOLINTNEXTLINE(readability-isolate-declaration)
    FD_ZERO(&readfds);

  // See https://sourceware.org/bugzilla/show_bug.cgi?id=12373.
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#endif

    // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-constant-array-index)
    FD_SET(fifo_fd_.get(), &readfds);

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

    constexpr std::chrono::seconds timeout{60};

    ::timeval tv{};
    tv.tv_sec = timeout.count();

    // On Linux if select() is interrupted by a signal handler, the
    // timeout is modified to indicate the remaining time. This means we
    // do not need to keep track of it. Note that this is not compliant
    // with SUSv3.
    int ready = detail::temp_failure_retry(::select, fifo_fd_.get() + 1,
                                           &readfds, nullptr, nullptr, &tv);

    if (ready == 0) {
        std::error_code err = std::make_error_code(std::errc::timed_out);
        throw std::system_error{err, fmt::format(
            "FIFO {0:n} of the SageMaker pipe channel timed out.", fifo_id_)};
    }

    if (ready == -1) {
        std::error_code err = current_error_code();
        throw std::system_error{err, fmt::format(
            "FIFO {0:n} of the SageMaker pipe channel cannot be read.",
            fifo_id_)};
    }
}

void
sagemaker_pipe_input_stream::
close() noexcept
{
    fifo_fd_ = {};

    detail::fifo_ids_[pathname_].exchange(fifo_id_);
}

void
sagemaker_pipe_input_stream::
check_if_closed() const
{
    if (fifo_fd_.is_open()) {
        return;
    }

    throw stream_error{"The input stream is closed."};
}

}  // namespace v1
}  // namespace mlio
