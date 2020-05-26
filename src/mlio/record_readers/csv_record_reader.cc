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

#include "mlio/record_readers/csv_record_reader.h"

#include <cstddef>

#include <fmt/format.h>

#include "mlio/csv_reader.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/detail/text_line.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_error.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

std::optional<Record>
Csv_record_reader::decode_text_record(Memory_slice &chunk, bool ignore_leftover)
{
    while (!chunk.empty()) {
        if (is_comment_line(chunk)) {
            if (detail::read_line(chunk, ignore_leftover) == std::nullopt) {
                break;
            }
        }
        else {
            std::optional<Record> record;
            if (params_->allow_quoted_new_lines) {
                record = read_line(chunk, ignore_leftover);
            }
            else {
                record = detail::read_line(chunk, ignore_leftover, params_->max_line_length);
            }

            if (record == std::nullopt) {
                return {};
            }

            if (!params_->skip_blank_lines || !record->payload().empty()) {
                return record;
            }
        }
    }

    return {};
}

inline bool Csv_record_reader::is_comment_line(const Memory_slice &chunk)
{
    if (params_->comment_char == std::nullopt) {
        return false;
    }

    auto chars = as_span<const char>(chunk);
    return !chars.empty() && chars[0] == *params_->comment_char;
}

std::optional<Record> Csv_record_reader::read_line(Memory_slice &chunk, bool ignore_leftover)
{
    auto chars = as_span<const char>(chunk);
    if (chars.empty()) {
        if (ignore_leftover) {
            return {};
        }

        throw Corrupt_record_error{"The text line ends with a corrupt character."};
    }

    auto pos = chars.begin();

    char chr{};

    Parser_state state;

new_field:
    state = Parser_state::new_field;

    if (!try_get_next_char(chars, pos, chr)) {
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
    state = Parser_state::in_field;

    if (!try_get_next_char(chars, pos, chr)) {
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
    state = Parser_state::in_quoted_field;

    if (!try_get_next_char(chars, pos, chr)) {
        goto end;  // NOLINT
    }

    if (chr == params_->quote_char) {
        goto quote_in_quoted_field;  // NOLINT
    }
    else {
        goto in_quoted_field;  // NOLINT
    }

quote_in_quoted_field:
    state = Parser_state::quote_in_quoted_field;

    if (!try_get_next_char(chars, pos, chr)) {
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
    state = Parser_state::has_carriage;

    if (!try_get_next_char(chars, pos, chr)) {
        goto end;  // NOLINT
    }

    // If we only have a carriage without a new-line character move back
    // to not lose the character we just read.
    if (chr != '\n') {
        state = Parser_state::new_field;

        --pos;
    }

    goto new_line;  // NOLINT

new_line : {
    if (params_->max_line_length) {
        check_line_length(chars, pos, *params_->max_line_length);
    }

    auto offset = sizeof(char) * as_size(pos - chars.begin());

    Memory_slice payload;
    if (state == Parser_state::has_carriage) {
        payload = chunk.first(offset - sizeof(char) * 2);
    }
    else {
        payload = chunk.first(offset - sizeof(char));
    }

    chunk = chunk.subslice(offset);

    return Record{std::move(payload)};
}

end:
    if (params_->max_line_length) {
        check_line_length(chars, pos, *params_->max_line_length);
    }

    if (ignore_leftover) {
        return {};
    }

    Memory_slice payload;

    switch (state) {
    case Parser_state::new_field:
    case Parser_state::in_field:
    case Parser_state::quote_in_quoted_field:
        payload = std::move(chunk);
        break;

    case Parser_state::has_carriage:
        payload = chunk.first(chunk.end() - sizeof(char));
        break;

    case Parser_state::in_quoted_field:
        throw Corrupt_record_error{"EOF reached inside a quoted field."};
    }

    chunk = {};

    return Record{std::move(payload)};
}

inline bool Csv_record_reader::try_get_next_char(const stdx::span<const char> &chars,
                                                 stdx::span<const char>::iterator &pos,
                                                 char &chr) noexcept
{
    if (pos == chars.end()) {
        return false;
    }

    chr = *pos;

    ++pos;

    return true;
}

inline void Csv_record_reader::check_line_length(const stdx::span<const char> &chars,
                                                 stdx::span<const char>::iterator &pos,
                                                 std::size_t max_line_length)
{
    std::size_t num_chars_read = as_size(pos - chars.begin());
    if (num_chars_read >= max_line_length) {
        throw Record_too_large_error{
            fmt::format("The text line exceeds the maximum length of {0:n}.", max_line_length)};
    }
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
