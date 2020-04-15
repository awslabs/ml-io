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
#include <string_view>
#include <utility>

#include <fmt/format.h>

#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_error.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {

std::optional<record>
read_line(memory_slice &chunk,
          bool ignore_leftover,
          std::optional<std::size_t> max_line_length)
{
    // Assumes chunk is not empty.
    auto chrs = as_span<char const>(chunk);
    if (chrs.empty()) {
        if (ignore_leftover) {
            return {};
        }

        throw corrupt_record_error{
            "The text line ends with a corrupt character."};
    }

    bool has_carriage = false;

    auto pos = chrs.begin();

    for (auto chr = *pos; pos < chrs.end(); ++pos, chr = *pos) {
        if (chr == '\n') {
            break;
        }

        if (chr == '\r') {
            auto next_pos = pos + 1;

            // Check if we have a new line with a new-line character
            // and make sure that we eat the carriage in such case.
            if (next_pos < chrs.end() && *next_pos == '\n') {
                has_carriage = true;

                pos = next_pos;
            }

            break;
        }
    }

    std::size_t num_chrs_read = as_size(pos - chrs.begin());

    if (max_line_length && num_chrs_read >= *max_line_length) {
        throw record_too_large_error{
            fmt::format("The text line exceeds the maximum length of {0:n}.",
                        *max_line_length)};
    }

    if (pos == chrs.end() && ignore_leftover) {
        return {};
    }

    auto offset = sizeof(char) * num_chrs_read;

    memory_slice payload;
    if (has_carriage) {
        payload = chunk.first(offset - sizeof(char));
    }
    else {
        payload = chunk.first(offset);
    }

    // Check if we reached the end of the stream or encountered a
    // new-line character.
    if (pos != chrs.end()) {
        // Skip the new-line character.
        offset += sizeof(char);

        chunk = chunk.subslice(offset);
    }
    else {
        chunk = {};
    }

    return record{std::move(payload), offset};
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
