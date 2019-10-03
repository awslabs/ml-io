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

// IWYU pragma: private, include "mlio/memory/file_mapped_memory_block.h"

#pragma once

#include <cstddef>
#include <string>

#include "mlio/byte.h"
#include "mlio/config.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup memory Memory
/// @{

/// Represents a file-mapped read-only memory block.
class MLIO_API file_mapped_memory_block final : public memory_block {
public:
    explicit
    file_mapped_memory_block(std::string pathname);

    file_mapped_memory_block(file_mapped_memory_block const &) = delete;

    file_mapped_memory_block(file_mapped_memory_block &&) = delete;

   ~file_mapped_memory_block() final;

public:
    file_mapped_memory_block &
    operator=(file_mapped_memory_block const &) = delete;

    file_mapped_memory_block &
    operator=(file_mapped_memory_block &&) = delete;

private:
    MLIO_HIDDEN
    void
    init_memory_map();

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
    std::string pathname_;
    stdx::byte *data_{};
    std::size_t size_{};
};

/// @}

}  // namespace v1
}  // namespace mlio
