/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "mlio/record_readers/blob_record_reader.h"

#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/record.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {

blob_record_reader::blob_record_reader(intrusive_ptr<input_stream> strm)
    : stream_record_reader{std::move(strm)}
{}

std::optional<record>
blob_record_reader::decode_record(memory_slice &chunk, bool)
{
    if (chunk.empty()) {
        return {};
    }
    auto payload = std::move(chunk);
    auto size = payload.size();
    chunk = {};
    return record{std::move(payload), size};
}

}  // namespace v1
}  // namespace mlio
