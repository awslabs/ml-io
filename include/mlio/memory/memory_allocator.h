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

#include <cstddef>
#include <memory>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup memory Memory
/// @{

class MLIO_API memory_allocator {
public:
    memory_allocator() noexcept = default;

    memory_allocator(memory_allocator const &) = delete;

    memory_allocator(memory_allocator &&) = delete;

    virtual
   ~memory_allocator();

public:
    memory_allocator &
    operator=(memory_allocator const &) = delete;

    memory_allocator &
    operator=(memory_allocator &&) = delete;

public:
    virtual intrusive_ptr<mutable_memory_block>
    allocate(std::size_t size) = 0;
};

/// Gets the default memory allocator.
MLIO_API
memory_allocator &
get_memory_allocator() noexcept;

/// Sets the default memory allocator.
MLIO_API
void
set_memory_allocator(std::unique_ptr<memory_allocator> &&alloc) noexcept;

/// @}

}  // namespace v1
}  // namespace mlio
