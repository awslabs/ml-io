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

/// Represents an unowned external memory block.
class MLIO_API external_memory_block final : public memory_block {
public:
    explicit
    external_memory_block(const_pointer data, size_type size) noexcept
        : data_{data}, size_{size}
    {}

    external_memory_block(external_memory_block const &) = delete;

    external_memory_block(external_memory_block &&) = delete;

   ~external_memory_block() override;

public:
    external_memory_block &
    operator=(external_memory_block const &) = delete;

    external_memory_block &
    operator=(external_memory_block &&) = delete;

public:
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

private:
    const_pointer data_;
    size_type size_;
};

/// @}

}  // namespace v1
}  // namespace mlio
