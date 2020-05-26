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

#include "mlio/memory/util.h"

#include <algorithm>
#include <utility>

#include "mlio/memory/memory_allocator.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace abi_v1 {

Intrusive_ptr<Mutable_memory_block>
resize_memory_block(Intrusive_ptr<Mutable_memory_block> &block, std::size_t size)
{
    if (block->resizable()) {
        block->resize(size);

        return std::move(block);
    }

    auto new_block = memory_allocator().allocate(size);

    if (size > block->size()) {
        size = block->size();
    }

    std::copy_n(block->begin(), size, new_block->begin());

    return new_block;
}

}  // namespace abi_v1
}  // namespace mlio
