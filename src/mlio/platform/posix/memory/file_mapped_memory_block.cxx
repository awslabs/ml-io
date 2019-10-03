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

#include "mlio/memory/file_mapped_memory_block.h"

#include <cstddef>
#include <utility>
#include <string>
#include <system_error>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mlio/byte.h"
#include "mlio/detail/error.h"
#include "mlio/detail/pathname.h"
#include "mlio/platform/posix/detail/file_descriptor.h"

using mlio::detail::file_descriptor;
using mlio::detail::current_error_code;

namespace mlio {
inline namespace v1 {

file_mapped_memory_block::
file_mapped_memory_block(std::string pathname)
    : pathname_{std::move(pathname)}
{
    detail::validate_file_pathname(pathname_);

    init_memory_map();
}

file_mapped_memory_block::~file_mapped_memory_block()
{
    if (data_ != nullptr) {
        ::munmap(data_, size_);
    }
}

void
file_mapped_memory_block::
init_memory_map()
{
#if !defined(_POSIX_C_SOURCE) || _POSIX_C_SOURCE >= 200809L
    file_descriptor fd = ::open(pathname_.c_str(), O_RDONLY | O_CLOEXEC);
#else
    file_descriptor fd = ::open(pathname_.c_str(), O_RDONLY);
#endif

    if (fd == -1) {
        throw std::system_error{current_error_code(),
            "The file cannot be opened."};
    }

    struct ::stat buf{};
    if (::fstat(fd.get(), &buf) == -1) {
        throw std::system_error{current_error_code(),
            "The size of the file cannot be retrieved."};
    }

    size_ = static_cast<std::size_t>(buf.st_size);
    if (size_ == 0) {
        return;
    }

    void *address = ::mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd.get(), 0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    if (address == MAP_FAILED) {
        throw std::system_error{current_error_code(),
            "The file cannot be memory mapped."};
    }

    data_ = static_cast<stdx::byte *>(address);
}

}  // namespace v1
}  // namespace mlio
