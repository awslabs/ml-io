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

#include "mlio/record_readers/detail/chunk_reader.h"

#include <utility>

#include "mlio/record_readers/detail/default_chunk_reader.h"
#include "mlio/record_readers/detail/in_memory_chunk_reader.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Chunk_reader::~Chunk_reader() = default;

std::unique_ptr<Chunk_reader> make_chunk_reader(Intrusive_ptr<Input_stream> stream)
{
    // See if we can zero-copy read the whole stream (e.g. a
    // memory-mapped file). In such case we can simply return a single
    // memory block right away instead of reading the stream chunk by
    // chunk.
    if (stream->supports_zero_copy()) {
        Memory_slice chunk = stream->read(stream->size());
        // Although this shouldn't happen, make sure we have read all
        // the data.
        if (chunk.size() == stream->size()) {
            return std::make_unique<In_memory_chunk_reader>(std::move(chunk));
        }

        stream->seek(0);
    }
    return std::make_unique<Default_chunk_reader>(std::move(stream));
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
