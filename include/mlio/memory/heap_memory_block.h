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

#include "mlio/config.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup memory Memory
/// @{

/// Represents a memory block allocated from the process heap.
class MLIO_API Heap_memory_block final : public Mutable_memory_block {
public:
    explicit Heap_memory_block(size_type size);

    Heap_memory_block(const Heap_memory_block &) = delete;

    Heap_memory_block &operator=(const Heap_memory_block &) = delete;

    Heap_memory_block(Heap_memory_block &&) = delete;

    Heap_memory_block &operator=(Heap_memory_block &&) = delete;

    ~Heap_memory_block() override;

    void resize(size_type size) final;

    pointer data() noexcept final
    {
        return data_;
    }

    const_pointer data() const noexcept final
    {
        return data_;
    }

    size_type size() const noexcept final
    {
        return size_;
    }

    bool resizable() const noexcept final
    {
        return true;
    }

private:
    pointer data_;
    size_type size_;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
