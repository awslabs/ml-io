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

#include "mlio/config.h"
#include "mlio/detail/file_descriptor.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup memory Memory
/// @{

/// Represents a memory block backed by a temporary file instead of the
/// process heap.
class MLIO_API file_backed_memory_block final : public mutable_memory_block {
public:
    explicit file_backed_memory_block(size_type size);

    file_backed_memory_block(const file_backed_memory_block &) = delete;

    file_backed_memory_block(file_backed_memory_block &&) = delete;

    ~file_backed_memory_block() final;

public:
    file_backed_memory_block &operator=(const file_backed_memory_block &) = delete;

    file_backed_memory_block &operator=(file_backed_memory_block &&) = delete;

public:
    void resize(size_type size) final;

private:
    MLIO_HIDDEN
    void make_temporary_file();

    MLIO_HIDDEN
    std::byte *init_memory_map(std::size_t size);

    MLIO_HIDDEN
    static void validate_mapped_address(void *addr);

public:
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
    detail::file_descriptor fd_{};
    std::byte *data_{};
    std::size_t size_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
