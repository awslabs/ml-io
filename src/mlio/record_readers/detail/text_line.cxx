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

#include "mlio/record_readers/detail/text_line.h"

#include <cstddef>
#include <utility>

#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/corrupt_record_error.h"
#include "mlio/record_readers/record.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {

stdx::optional<record>
read_line(memory_slice &chunk, bool ignore_leftover)
{
    // Assumes chunk is not empty.
    auto chrs = as_span<char const>(chunk);
    if (chrs.empty()) {
        if(ignore_leftover) {
            return {};
        }

        throw corrupt_record_error{"The line ends with a corrupt character."};
    }

    bool has_carriage = false;

    auto pos = chrs.begin();

    for (auto chr = *pos; pos != chrs.end(); ++pos, chr = *pos) {
        if (chr == '\n' || has_carriage) {
            if (chr != '\n') {
                has_carriage = false;

                // We have a new line without a line-feed, make sure we
                // move back and do not lose the character we just read.
                --pos;
            }

            break;
        }

        if (chr == '\r') {
            has_carriage = true;
        }
    }

    if (pos == chrs.end() && ignore_leftover) {
        return {};
    }

    auto offset = sizeof(char) * as_size(pos - chrs.begin());

    memory_slice payload;
    if (has_carriage) {
        payload = chunk.first(offset - sizeof(char));
    } else {
        payload = chunk.first(offset);
    }

    if (pos != chrs.end()) {
        chunk = chunk.subslice(offset + sizeof(char));
    } else {
        chunk = {};
    }

    return record{std::move(payload)};
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
