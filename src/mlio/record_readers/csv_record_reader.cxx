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

#include <cstddef>
#include <string_view>

#include <fmt/format.h>

#include "mlio/config.h"
#include "mlio/csv_reader.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/detail/text_line.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_error.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {

std::optional<record>
csv_record_reader::decode_text_record(memory_slice &chunk,
                                      bool ignore_leftover)
{
    while (!chunk.empty()) {
        if (is_comment_line(chunk)) {
            if (detail::read_line(chunk, ignore_leftover) == std::nullopt) {
                break;
            }
        }
        else {
            std::optional<record> rec;
            if (params_->allow_quoted_new_lines) {
                rec = read_line(chunk, ignore_leftover);
            }
            else {
                rec = detail::read_line(
                    chunk, ignore_leftover, params_->max_line_length);
            }

            if (rec == std::nullopt) {
                return {};
            }

            if (!params_->skip_blank_lines || !rec->payload().empty()) {
                return rec;
            }
        }
    }

    return {};
}

inline bool
csv_record_reader::is_comment_line(memory_slice const &chunk)
{
    if (params_->comment_char == std::nullopt) {
        return false;
    }

    auto chrs = as_span<char const>(chunk);
    return !chrs.empty() && chrs[0] == *params_->comment_char;
}

std::optional<record>
csv_record_reader::read_line(memory_slice &chunk, bool ignore_leftover)
{
    auto chrs = as_span<char const>(chunk);
    if (chrs.empty()) {
        if (ignore_leftover) {
            return {};
        }

        throw corrupt_record_error{
            "The text line ends with a corrupt character."};
    }

    auto pos = chrs.begin();

    char chr{};

    parser_state state;

new_field:
    state = parser_state::new_field;

    if (!try_get_next_char(chrs, pos, chr)) {
        goto end;  // NOLINT
    }

    if (chr == params_->delimiter) {
        goto new_field;  // NOLINT
    }
    else if (chr == params_->quote_char) {
        goto in_quoted_field;  // NOLINT
    }
    else if (chr == '\n') {
        goto new_line;  // NOLINT
    }
    else if (chr == '\r') {
        goto has_carriage;  // NOLINT
    }
    else {
        goto in_field;  // NOLINT
    }

in_field:
    state = parser_state::in_field;

    if (!try_get_next_char(chrs, pos, chr)) {
        goto end;  // NOLINT
    }

    if (chr == params_->delimiter) {
        goto new_field;  // NOLINT
    }
    else if (chr == '\n') {
        goto new_line;  // NOLINT
    }
    else if (chr == '\r') {
        goto has_carriage;  // NOLINT
    }
    else {
        goto in_field;  // NOLINT
    }

in_quoted_field:
    state = parser_state::in_quoted_field;

    if (!try_get_next_char(chrs, pos, chr)) {
        goto end;  // NOLINT
    }

    if (chr == params_->quote_char) {
        goto quote_in_quoted_field;  // NOLINT
    }
    else {
        goto in_quoted_field;  // NOLINT
    }

quote_in_quoted_field:
    state = parser_state::quote_in_quoted_field;

    if (!try_get_next_char(chrs, pos, chr)) {
        goto end;  // NOLINT
    }

    if (chr == params_->delimiter) {
        goto new_field;  // NOLINT
    }
    else if (chr == params_->quote_char) {
        goto in_quoted_field;  // NOLINT
    }
    else if (chr == '\n') {
        goto new_line;  // NOLINT
    }
    else if (chr == '\r') {
        goto has_carriage;  // NOLINT
    }
    else {
        goto in_field;  // NOLINT
    }

has_carriage:
    state = parser_state::has_carriage;

    if (!try_get_next_char(chrs, pos, chr)) {
        goto end;  // NOLINT
    }

    // If we only have a carriage without a new-line character move back
    // to not lose the character we just read.
    if (chr != '\n') {
        state = parser_state::new_field;

        --pos;
    }

    goto new_line;  // NOLINT

new_line : {
    if (params_->max_line_length) {
        check_line_length(chrs, pos, *params_->max_line_length);
    }

    auto offset = sizeof(char) * as_size(pos - chrs.begin());

    memory_slice payload;
    if (state == parser_state::has_carriage) {
        payload = chunk.first(offset - sizeof(char) * 2);
    }
    else {
        payload = chunk.first(offset - sizeof(char));
    }

    chunk = chunk.subslice(offset);

    return record{std::move(payload), offset};
}

end:
    if (params_->max_line_length) {
        check_line_length(chrs, pos, *params_->max_line_length);
    }

    if (ignore_leftover) {
        return {};
    }

    std::size_t size = chunk.size();

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
    }

    chunk = {};

    return record{std::move(payload), size};
}

inline bool
csv_record_reader::try_get_next_char(stdx::span<char const> const &chrs,
                                     stdx::span<char const>::iterator &pos,
                                     char &chr) noexcept
{
    if (pos == chrs.end()) {
        return false;
    }

    chr = *pos;

    ++pos;

    return true;
}

inline void
csv_record_reader::check_line_length(stdx::span<char const> const &chrs,
                                     stdx::span<char const>::iterator &pos,
                                     std::size_t max_line_length)
{
    std::size_t num_chrs_read = as_size(pos - chrs.begin());
    if (num_chrs_read >= max_line_length) {
        throw record_too_large_error{
            fmt::format("The text line exceeds the maximum length of {0:n}.",
                        max_line_length)};
    }
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
