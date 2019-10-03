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

#include "mlio/byte.h"
#include "mlio/config.h"
#include "mlio/intrusive_ref_counter.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup memory Memory
/// @{

class MLIO_API memory_block : public intrusive_ref_counter<memory_block> {
public:
    using value_type = stdx::byte;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = stdx::byte const &;
    using const_reference = stdx::byte const &;
    using pointer = stdx::byte const *;
    using const_pointer = stdx::byte const *;
    using iterator = stdx::byte const *;
    using const_iterator = stdx::byte const *;

public:
    memory_block() noexcept = default;

    memory_block(memory_block const &) = delete;

    memory_block(memory_block &&) = delete;

    virtual
   ~memory_block();

public:
    memory_block &
    operator=(memory_block const &) = delete;

    memory_block &
    operator=(memory_block &&) = delete;

public:
    const_iterator
    begin() const noexcept
    {
        return data();
    }

    const_iterator
    end() const noexcept
    {
        return data() + size();
    }

    const_iterator
    cbegin() const noexcept
    {
        return begin();
    }

    const_iterator
    cend() const noexcept
    {
        return end();
    }

public:
    virtual const_pointer
    data() const noexcept = 0;

    virtual size_type
    size() const noexcept = 0;
};

class MLIO_API mutable_memory_block : public memory_block {
public:
    using reference = stdx::byte &;
    using pointer = stdx::byte *;
    using iterator = stdx::byte *;

public:
    mutable_memory_block() noexcept = default;

    mutable_memory_block(mutable_memory_block const &) = delete;

    mutable_memory_block(mutable_memory_block &&) = delete;

   ~mutable_memory_block() override;

public:
    mutable_memory_block &
    operator=(mutable_memory_block const &) = delete;

    mutable_memory_block &
    operator=(mutable_memory_block &&) = delete;

public:
    iterator
    begin() noexcept
    {
        return data();
    }

    iterator
    end() noexcept
    {
        return data() + size();
    }

    virtual void
    resize(size_type size) = 0;

public:
    using memory_block::data;

    virtual pointer
    data() noexcept = 0;

    virtual bool
    resizable() const noexcept = 0;
};

/// @}

}  // namespace v1
}  // namespace mlio
