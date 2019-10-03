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

#include "mlio/record_readers/csv_record_reader.h"

#include <cassert>
#include <cstddef>

#include "mlio/config.h"
#include "mlio/csv_reader.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/corrupt_record_error.h"
#include "mlio/record_readers/detail/text_line.h"
#include "mlio/record_readers/record.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {

stdx::optional<record>
csv_record_reader::
decode_text_record(memory_slice &chunk, bool ignore_leftover)
{
    while (!chunk.empty()) {
        if (is_comment_line(chunk)) {
            if (detail::read_line(chunk, ignore_leftover) == stdx::nullopt) {
                break;
            }
        } else {
            stdx::optional<record> rec;
            if (params_->allow_quoted_new_lines) {
                rec = read_line(chunk, ignore_leftover);
            } else {
                rec = detail::read_line(chunk, ignore_leftover);
            }

            if (rec == stdx::nullopt) {
                return rec;
            }

            if (!params_->skip_blank_lines || !rec->payload().empty()) {
                return rec;
            }
        }
    }

    return {};
}

bool
csv_record_reader::
is_comment_line(memory_slice const &chunk)
{
    if (params_->comment_char == stdx::nullopt) {
        return false;
    }

    auto chrs = as_span<char const>(chunk);
    return !chrs.empty() && chrs[0] == *params_->comment_char;
}

stdx::optional<record>
csv_record_reader::
read_line(memory_slice &chunk, bool ignore_leftover)
{
    auto chrs = as_span<char const>(chunk);
    if (chrs.empty()) {
        if (ignore_leftover) {
            return {};
        }

        throw corrupt_record_error{"The line ends with a corrupt character."};
    }

    bool has_carriage = false;

    auto state = parser_state::new_line;

    for(auto pos = chrs.begin(); pos != chrs.end(); ++pos) {
        auto chr = *pos;

        switch (state) {
        case parser_state::new_line:
            state = parser_state::new_field;

            MLIO_FALLTHROUGH;
        case parser_state::new_field:
            if (chr == params_->delimiter) {
                state = parser_state::new_field;
            } else if (chr == '\r') {
                state = parser_state::has_carriage;
            } else if (chr == '\n') {
                state = parser_state::new_line;
            } else if (chr == '"') {
                state = parser_state::in_quoted_field;
            } else {
                state = parser_state::in_field;
            }
            break;

        case parser_state::in_field:
            if (chr == params_->delimiter) {
                state = parser_state::new_field;
            } else if (chr == '\r') {
                state = parser_state::has_carriage;
            } else if (chr == '\n') {
                state = parser_state::new_line;
            }
            break;

        case parser_state::in_quoted_field:
            if (chr == '"') {
                state = parser_state::quote_in_quoted_field;
            }
            break;

        case parser_state::quote_in_quoted_field:
            if (chr == '"') {
                state = parser_state::in_quoted_field;
            } else if (chr == params_->delimiter) {
                state = parser_state::new_field;
            } else if (chr == '\r') {
                state = parser_state::has_carriage;
            } else if (chr == '\n') {
                state = parser_state::new_line;
            } else {
                state = parser_state::in_field;
            }
            break;

        case parser_state::has_carriage:
            if (chr == '\n') {
                has_carriage = true;
            } else {
                has_carriage = false;

                --pos;
            }

            state = parser_state::new_line;

            break;
        }

        if (state == parser_state::new_line) {
            auto offset = sizeof(char) * as_size(pos - chrs.begin());

            memory_slice payload;
            if (has_carriage) {
                payload = chunk.first(offset - sizeof(char));
            } else {
                payload = chunk.first(offset);
            }

            chunk = chunk.subslice(offset + sizeof(char));

            return record{std::move(payload)};
        }
    }

    if (ignore_leftover) {
        return {};
    }

    memory_slice payload;

    switch (state) {
    case parser_state::new_field:
    case parser_state::in_field:
    case parser_state::quote_in_quoted_field:
        payload = std::move(chunk);
        break;

    case parser_state::has_carriage:
        payload = chunk.first(chunk.end() - sizeof(char));
        break;

    case parser_state::in_quoted_field:
        throw corrupt_record_error{"EOF reached inside a quoted field."};

    case parser_state::new_line:
        assert(false);
    }

    chunk = {};

    return record{std::move(payload)};
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
