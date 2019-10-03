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

#include "mlio/memory/heap_memory_block.h"

#include <cstddef>
#include <cstdlib>
#include <new>

#include "mlio/byte.h"

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

stdx::byte *
allocate_data(std::size_t size, stdx::byte *old_data = nullptr)
{
    void *data = ::realloc(old_data, size);  // NOLINT
    if (data == nullptr) {
        throw std::bad_alloc{};
    }
    return static_cast<stdx::byte *>(data);
}

void
free_data(stdx::byte *data)
{
    ::free(data);  // NOLINT
}

}  // namespace
}  // namespace detail

heap_memory_block::
heap_memory_block(size_type size)
    : size_{size}
{
    if (size_ == 0) {
        data_ = nullptr;
    } else {
        data_ = detail::allocate_data(size);
    }
}

heap_memory_block::~heap_memory_block()
{
    detail::free_data(data_);
}

void
heap_memory_block::
resize(size_type size)
{
    if (size == 0) {
        detail::free_data(data_);

        data_ = nullptr;
        size_ = 0;
    } else {
        data_ = detail::allocate_data(size, data_);
        size_ = size;
    }
}

}  // namespace v1
}  // namespace mlio
