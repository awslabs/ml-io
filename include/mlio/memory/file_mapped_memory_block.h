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
#include <string>

#include "mlio/config.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup memory Memory
/// @{

/// Represents a File-mapped read-only memory block.
class MLIO_API File_mapped_memory_block final : public Memory_block {
public:
    explicit File_mapped_memory_block(std::string path);

    File_mapped_memory_block(const File_mapped_memory_block &) = delete;

    File_mapped_memory_block &operator=(const File_mapped_memory_block &) = delete;

    File_mapped_memory_block(File_mapped_memory_block &&) = delete;

    File_mapped_memory_block &operator=(File_mapped_memory_block &&) = delete;

    ~File_mapped_memory_block() final;

    const_pointer data() const noexcept final
    {
        return data_;
    }

    size_type size() const noexcept final
    {
        return size_;
    }

private:
    MLIO_HIDDEN
    void init_memory_map();

    std::string path_;
    std::byte *data_{};
    std::size_t size_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
