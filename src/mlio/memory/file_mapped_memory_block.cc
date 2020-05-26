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

#include "mlio/memory/file_mapped_memory_block.h"

#include <cstddef>
#include <system_error>
#include <utility>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "mlio/detail/error.h"
#include "mlio/detail/file_descriptor.h"
#include "mlio/detail/path.h"

using mlio::detail::current_error_code;
using mlio::detail::File_descriptor;

namespace mlio {
inline namespace abi_v1 {

File_mapped_memory_block::File_mapped_memory_block(std::string path) : path_{std::move(path)}
{
    detail::validate_file_path(path_);

    init_memory_map();
}

File_mapped_memory_block::~File_mapped_memory_block()
{
    if (data_ != nullptr) {
        ::munmap(data_, size_);
    }
}

void File_mapped_memory_block::init_memory_map()
{
    File_descriptor fd = ::open(path_.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd == -1) {
        throw std::system_error{current_error_code(), "The file cannot be opened."};
    }

    struct ::stat buf {};
    if (::fstat(fd.get(), &buf) == -1) {
        throw std::system_error{current_error_code(), "The size of the file cannot be retrieved."};
    }

    size_ = static_cast<std::size_t>(buf.st_size);
    if (size_ == 0) {
        return;
    }

    void *address = ::mmap(nullptr, size_, PROT_READ, MAP_PRIVATE, fd.get(), 0);
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    if (address == MAP_FAILED) {
        throw std::system_error{current_error_code(), "The file cannot be memory mapped."};
    }

    data_ = static_cast<std::byte *>(address);
}

}  // namespace abi_v1
}  // namespace mlio
