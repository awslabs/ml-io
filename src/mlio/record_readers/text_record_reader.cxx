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

#include "mlio/record_readers/text_record_reader.h"

#include <utility>

#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/record.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {

text_record_reader::
text_record_reader(intrusive_ptr<input_stream> strm)
    : stream_record_reader{std::move(strm)}
{}

stdx::optional<record>
text_record_reader::
decode_record(memory_slice &chunk, bool ignore_leftover)
{
    if (chunk.empty()) {
        return {};
    }

    if (skip_utf8_bom(chunk, ignore_leftover)) {
        return decode_text_record(chunk, ignore_leftover);
    }

    return {};
}

bool
text_record_reader::
skip_utf8_bom(memory_slice &chunk, bool ignore_leftover) noexcept
{
    auto bits = as_span<unsigned char const>(chunk);
    if (bits.size() >= 3) {
        if (bits[0] == 0xEF && bits[1] == 0xBB && bits[2] == 0xBF) {
            chunk = chunk.subslice(sizeof(unsigned char) * 3);
        }
    } else {
        if (ignore_leftover) {
            return false;
        }
    }
    return true;
}

}  // namespace v1
}  // namespace mlio
