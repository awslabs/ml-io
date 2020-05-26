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

#include <functional>
#include <utility>

#include <unistd.h>

#include "mlio/config.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class MLIO_API File_descriptor {
public:
    File_descriptor() noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    File_descriptor(int fd) noexcept : fd_{fd}
    {}

    File_descriptor(const File_descriptor &other) = delete;

    File_descriptor &operator=(const File_descriptor &other) = delete;

    File_descriptor(File_descriptor &&other) noexcept : fd_{other.fd_}
    {
        other.fd_ = invalid_fd_;
    }

    File_descriptor &operator=(File_descriptor &&other) noexcept
    {
        close_fd();

        fd_ = std::exchange(other.fd_, invalid_fd_);

        return *this;
    }

    ~File_descriptor()
    {
        close_fd();
    }

    int get() const noexcept
    {
        return fd_;
    }

    bool is_open() const noexcept
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

    static constexpr int invalid_fd_ = -1;

    int fd_ = invalid_fd_;
};

MLIO_API
inline bool operator==(const File_descriptor &lhs, const File_descriptor &rhs) noexcept
{
    return lhs.get() == rhs.get();
}

MLIO_API
inline bool operator!=(const File_descriptor &lhs, const File_descriptor &rhs) noexcept
{
    return lhs.get() != rhs.get();
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::detail::File_descriptor> {
    inline size_t operator()(const mlio::detail::File_descriptor &desc) const noexcept
    {
        return hash<int>{}(desc.get());
    }
};

}  // namespace std
