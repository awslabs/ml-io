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

#include "mlio/memory/file_backed_memory_block.h"

#include <cerrno>
#include <cstdlib>
#include <system_error>

#include <sys/mman.h>
#include <unistd.h>

#include "mlio/config.h"
#include "mlio/detail/error.h"

using mlio::detail::current_error_code;

namespace mlio {
inline namespace v1 {

file_backed_memory_block::
file_backed_memory_block(size_type size)
    : size_{size}
{
    make_temporary_file();

    if (size_ == 0) {
        data_ = nullptr;
    } else {
        data_ = init_memory_map(size_);
    }
}

file_backed_memory_block::~file_backed_memory_block()
{
    if (data_ != nullptr) {
        ::munmap(data_, size_);
    }
}

void
file_backed_memory_block::
make_temporary_file()
{
    std::string pathname{"/tmp/mlio-XXXXXX"};

    fd_ = ::mkstemp(&pathname.front());

    auto off = static_cast<::off_t>(size_);
    if (fd_ == -1 || ::ftruncate(fd_.get(), off) != 0) {
        throw std::system_error{current_error_code(),
            "The file-backed memory block cannot be allocated."};
    }

    ::unlink(&pathname.front());
}

void
file_backed_memory_block::
resize(size_type size)
{
    if (size == size_) {
        return;
    }

    auto off = static_cast<::off_t>(size);
    if (::ftruncate(fd_.get(), off) != 0) {
        throw std::system_error{current_error_code(),
            "The file-backed memory block cannot be resized."};
    }

    if (size_ == 0) {
        data_ = init_memory_map(size);
        size_ = size;
    } else if (size == 0) {
        ::munmap(data_, size_);

        data_ = nullptr;
        size_ = 0;
    } else {
#ifdef MLIO_PLATFORM_LINUX
        void *addr = ::mremap(data_, size_, size, MREMAP_MAYMOVE);

        validate_mapped_address(addr);

        data_ = static_cast<stdx::byte *>(addr);
        size_ = size;
#else
        stdx::byte *data;
        try {
            data = init_memory_map(size);
        }
        catch (std::system_error const &) {
            // We already truncated the backing file, but cannot map it
            // to memory. If the new size is less than the original size
            // this means we lost the data in the truncated region. In
            // such case we should abort the process as there is no way
            // to recover gracefully.
            if (size_ > size) {
                std::abort();
            }

            throw;
        }

        ::munmap(data_, size_);

        data_ = data;
        size_ = size;
#endif
    }
}

stdx::byte *
file_backed_memory_block::
init_memory_map(std::size_t size)
{
    void *addr = ::mmap(/*addr*/ nullptr, size, PROT_READ | PROT_WRITE,
                        MAP_SHARED, fd_.get(), /*offset*/ 0);

    validate_mapped_address(addr);

    return static_cast<stdx::byte *>(addr);
}

void
file_backed_memory_block::
validate_mapped_address(void *addr)
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-cstyle-cast)
    if (addr != MAP_FAILED) {
        return;
    }

    if (errno == ENOMEM) {
        throw std::bad_alloc{};
    }
    throw std::system_error{current_error_code(),
        "The file-backed memory block cannot be mapped."};
}

}  // namespace v1
}  // namespace mlio
