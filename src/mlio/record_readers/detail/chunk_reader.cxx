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

#include "mlio/record_readers/detail/chunk_reader.h"

#include <utility>

#include "mlio/streams/input_stream.h"
#include "mlio/record_readers/detail/default_chunk_reader.h"
#include "mlio/record_readers/detail/in_memory_chunk_reader.h"

namespace mlio {
inline namespace v1 {
namespace detail {

chunk_reader::~chunk_reader() = default;

std::unique_ptr<chunk_reader>
make_chunk_reader(intrusive_ptr<input_stream> strm)
{
    // See if we can zero-copy read the whole stream (e.g. a
    // memory-mapped file). In such case we can simply return a single
    // memory block right away instead of reading chunk by chunk.
    if (strm->supports_zero_copy()) {
        memory_slice chunk = strm->read(strm->size());
        if (chunk.size() == strm->size()) {
            return std::make_unique<in_memory_chunk_reader>(std::move(chunk));
        }

        strm->seek(0);
    }
    return std::make_unique<default_chunk_reader>(std::move(strm));
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
