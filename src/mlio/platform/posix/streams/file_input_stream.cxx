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

#include "mlio/streams/file_input_stream.h"  // IWYU pragma: associated

#include <algorithm>
#include <system_error>
#include <utility>

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mlio/detail/error.h"
#include "mlio/detail/pathname.h"
#include "mlio/logger.h"
#include "mlio/streams/stream_error.h"

using mlio::detail::current_error_code;

namespace mlio {
inline namespace v1 {

file_input_stream::
file_input_stream(std::string pathname)
    : pathname_{std::move(pathname)}
{
    detail::validate_file_pathname(pathname_);

#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE >= 200809L
    fd_ = ::open(pathname_.c_str(), O_RDONLY | O_CLOEXEC);
#else
    fd_ = ::open(pathname_.c_str(), O_RDONLY);
#endif

    if (fd_ == -1) {
        throw std::system_error{current_error_code(),
            "The file cannot be opened."};
    }

#ifdef MLIO_PLATFORM_LINUX
    int r = ::posix_fadvise(fd_.get(), 0, 0, POSIX_FADV_SEQUENTIAL);
    if (r != 0) {
        logger::warn(
            "The read-ahead size of the file '{0}' cannot be increased.",
            pathname_);
    }
#endif
}

std::size_t
file_input_stream::
read(mutable_memory_span dest)
{
    check_if_closed();

    if (dest.empty()) {
        return 0;
    }

    ssize_t num_bytes_read = ::read(fd_.get(), dest.data(), dest.size());
    if (num_bytes_read == -1) {
        throw std::system_error{current_error_code(),
            "The file cannot be read."};
    }
    return static_cast<std::size_t>(num_bytes_read);
}

void
file_input_stream::
seek(std::size_t position)
{
    check_if_closed();

    auto offset = static_cast<::off_t>(std::min(position, size()));

    ::off_t o = ::lseek(fd_.get(), offset, SEEK_SET);
    if (o == -1) {
        std::error_code err = current_error_code();

        char const *msg;
        if (err == std::errc::invalid_seek) {
            msg = "The file is not seekable.";
        } else {
            msg = "The position in the file cannot be set.";
        }

        throw std::system_error{err, msg};
    }
}

void
file_input_stream::
close() noexcept
{
    fd_ = {};
}

void
file_input_stream::
check_if_closed() const
{
    if (fd_.is_open()) {
        return;
    }

    throw stream_error{"The input stream is closed."};
}

std::size_t
file_input_stream::
size() const
{
    check_if_closed();

    if (size_ == 0) {
        struct ::stat buf{};
        if (::fstat(fd_.get(), &buf) == -1) {
            throw std::system_error{current_error_code(),
                "The size of the file cannot be retrieved."};
        }

        size_ = static_cast<std::size_t>(buf.st_size);
    }
    return size_;
}

std::size_t
file_input_stream::
position() const
{
    check_if_closed();

    ::off_t o = ::lseek(fd_.get(), 0, SEEK_CUR);
    if (o == -1) {
        std::error_code err = current_error_code();

        char const *msg;
        if (err == std::errc::invalid_seek) {
            msg = "The file is not seekable.";
        } else {
            msg = "The position in the file cannot be retrieved.";
        }

        throw std::system_error{err, msg};
    }
    return static_cast<std::size_t>(o);
}

}  // namespace v1
}  // namespace mlio
