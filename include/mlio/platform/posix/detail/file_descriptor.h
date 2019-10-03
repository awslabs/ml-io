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

#include <functional>
#include <utility>

#include <unistd.h>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class MLIO_API file_descriptor {
private:
    static constexpr int invalid_fd_ = -1;

public:
    file_descriptor() noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    file_descriptor(int fd) noexcept
        : fd_{fd}
    {}

    file_descriptor(file_descriptor const &other) = delete;

    file_descriptor(file_descriptor &&other) noexcept
        : fd_{other.fd_}
    {
        other.fd_ = invalid_fd_;
    }

   ~file_descriptor()
    {
        close_fd();
    }

public:
    file_descriptor &
    operator=(file_descriptor const &other) = delete;

    file_descriptor &
    operator=(file_descriptor &&other) noexcept
    {
        close_fd();

        fd_ = other.fd_;
        other.fd_ = invalid_fd_;

        return *this;
    }

public:
    int
    get() const noexcept
    {
        return fd_;
    }

    bool
    is_open() const noexcept
    {
        return fd_ != invalid_fd_;
    }

private:
    void close_fd() noexcept
    {
        if (fd_ == invalid_fd_) {
            return;
        }

        ::close(fd_);

        fd_ = invalid_fd_;
    }

private:
    int fd_ = invalid_fd_;
};

MLIO_API
inline bool
operator==(file_descriptor const &lhs, file_descriptor const &rhs) noexcept
{
    return lhs.get() == rhs.get();
}

MLIO_API
inline bool
operator!=(file_descriptor const &lhs, file_descriptor const &rhs) noexcept
{
    return lhs.get() != rhs.get();
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::detail::file_descriptor> {
    inline size_t
    operator()(mlio::detail::file_descriptor const &desc) const noexcept
    {
        return hash<int>{}(desc.get());
    }
};

}  // namespace std
