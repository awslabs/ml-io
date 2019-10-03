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

#include "mlio/config.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup memory Memory
/// @{

/// Represents a memory block allocated from the process heap.
class MLIO_API heap_memory_block final : public mutable_memory_block {
public:
    explicit
    heap_memory_block(size_type size);

    heap_memory_block(heap_memory_block const &) = delete;

    heap_memory_block(heap_memory_block &&) = delete;

   ~heap_memory_block() override;

public:
    heap_memory_block &
    operator=(heap_memory_block const &) = delete;

    heap_memory_block &
    operator=(heap_memory_block &&) = delete;

public:
    void
    resize(size_type size) final;

public:
    pointer
    data() noexcept final
    {
        return data_;
    }

    const_pointer
    data() const noexcept final
    {
        return data_;
    }

    size_type
    size() const noexcept final
    {
        return size_;
    }

    bool
    resizable() const noexcept final
    {
        return true;
    }

private:
    pointer data_;
    size_type size_;
};

/// @}

}  // namespace v1
}  // namespace mlio
