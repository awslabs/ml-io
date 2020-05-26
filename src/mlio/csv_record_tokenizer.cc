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

#include "mlio/csv_record_tokenizer.h"

#include "mlio/record_readers/record_error.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

bool Csv_record_tokenizer::next()
{
    value_.clear();

    truncated_ = false;

    if (finished_) {
        eof_ = true;

        return false;
    }

    char chr{};

    // Start of a new field.
    Parser_state state = Parser_state::new_field;

    if (!try_get_next_char(chr)) {
        goto end_line;  // NOLINT
    }

    if (chr == delimiter_) {
        goto end_field;  // NOLINT
    }
    else if (chr == quote_char_) {
        goto in_quoted_field;  // NOLINT
    }
    else {
        push_char(chr);
        goto in_field;  // NOLINT
    }

in_field:
    state = Parser_state::in_field;

    if (!try_get_next_char(chr)) {
        goto end_line;  // NOLINT
    }

    if (chr == delimiter_) {
        goto end_field;  // NOLINT
    }
    else {
        push_char(chr);
        goto in_field;  // NOLINT
    }

in_quoted_field:
    state = Parser_state::in_quoted_field;

    if (!try_get_next_char(chr)) {
        goto end_line;  // NOLINT
    }

    if (chr == quote_char_) {
        goto quote_in_quoted_field;  // NOLINT
    }
    else {
        push_char(chr);
        goto in_quoted_field;  // NOLINT
    }

quote_in_quoted_field:
    state = Parser_state::quote_in_quoted_field;

    if (!try_get_next_char(chr)) {
        goto end_line;  // NOLINT
    }

    if (chr == delimiter_) {
        goto end_field;  // NOLINT
    }
    else if (chr == quote_char_) {
        push_char(chr);
        goto in_quoted_field;  // NOLINT
    }
    else {
        push_char(chr);
        goto in_field;  // NOLINT
    }

end_line:
    switch (state) {
    case Parser_state::new_field:
    case Parser_state::in_field:
    case Parser_state::quote_in_quoted_field:
        finished_ = true;
        break;

    case Parser_state::in_quoted_field:
        throw Corrupt_record_error{"EOF reached inside a quoted field."};
    }

end_field:
    return true;
}

inline bool Csv_record_tokenizer::try_get_next_char(char &chr) noexcept
{
    if (text_pos_ == text_.end()) {
        return false;
    }

    chr = *text_pos_;

    ++text_pos_;

    return true;
}

inline void Csv_record_tokenizer::push_char(char chr) noexcept
{
    if (max_field_length_ && value_.size() == *max_field_length_) {
        truncated_ = true;
    }
    else {
        value_.push_back(chr);
    }
}

void Csv_record_tokenizer::reset(Memory_span blob)
{
    text_ = as_span<const char>(blob);

    text_pos_ = text_.begin();

    finished_ = false;

    eof_ = false;
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
