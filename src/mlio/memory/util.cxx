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

#include "mlio/memory/util.h"

#include <algorithm>
#include <utility>

#include "mlio/memory/memory_allocator.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace v1 {

intrusive_ptr<mutable_memory_block>
resize_memory_block(intrusive_ptr<mutable_memory_block> &blk, std::size_t size)
{
    if (blk->resizable()) {
        blk->resize(size);

        return std::move(blk);
    }

    auto new_blk = get_memory_allocator().allocate(size);

    if (size > blk->size()) {
        size = blk->size();
    }

    std::copy_n(blk->begin(), size, new_blk->begin());

    return new_blk;
}

}  // namespace v1
}  // namespace mlio
