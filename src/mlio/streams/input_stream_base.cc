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

#include "mlio/streams/input_stream_base.h"

#include <utility>

#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_allocator.h"
#include "mlio/memory/memory_block.h"
#include "mlio/not_supported_error.h"

namespace mlio {
inline namespace abi_v1 {

Memory_slice Input_stream_base::read(std::size_t size)
{
    auto block = memory_allocator().allocate(size);

    std::size_t num_bytes_read = read(*block);

    return Memory_slice{std::move(block)}.first(num_bytes_read);
}

void Input_stream_base::seek(std::size_t)
{
    throw Not_supported_error{"The input stream is not seekable."};
}

std::size_t Input_stream_base::size() const
{
    throw Not_supported_error{"The input stream is not seekable."};
}

std::size_t Input_stream_base::position() const
{
    throw Not_supported_error{"The input stream is not seekable."};
}

}  // namespace abi_v1
}  // namespace mlio
