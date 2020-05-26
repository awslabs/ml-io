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

#include <cstddef>
#include <memory>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup memory Memory
/// @{

class MLIO_API Memory_allocator {
public:
    Memory_allocator() noexcept = default;

    Memory_allocator(const Memory_allocator &) = delete;

    Memory_allocator &operator=(const Memory_allocator &) = delete;

    Memory_allocator(Memory_allocator &&) = delete;

    Memory_allocator &operator=(Memory_allocator &&) = delete;

    virtual ~Memory_allocator();

    virtual Intrusive_ptr<Mutable_memory_block> allocate(std::size_t size) = 0;
};

/// Gets the default memory allocator.
MLIO_API
Memory_allocator &memory_allocator() noexcept;

/// Sets the default memory allocator.
MLIO_API
void set_memory_allocator(std::unique_ptr<Memory_allocator> &&allocator) noexcept;

/// @}

}  // namespace abi_v1
}  // namespace mlio
