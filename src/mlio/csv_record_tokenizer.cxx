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

#include "mlio/csv_record_tokenizer.h"

#include <cassert>

#include "mlio/config.h"
#include "mlio/record_readers/corrupt_record_error.h"

namespace mlio {
inline namespace v1 {
namespace detail {

bool
csv_record_tokenizer::
next()
{
    if (state_ == parser_state::finished) {
        eof_ = true;

        return false;
    }

    while (pos_ != text_.end()) {
        char chr = *pos_;

        switch(state_) {
        case parser_state::end_field:
            state_ = parser_state::new_field;

            MLIO_FALLTHROUGH;
        case parser_state::new_field:
            value_.clear();

            if (chr == delimiter_) {
                state_ = parser_state::end_field;
            } else if (chr == '"') {
                state_ = parser_state::in_quoted_field;
            } else {
                value_.push_back(chr);
                state_ = parser_state::in_field;
            }
            break;

        case parser_state::in_field:
            if (chr == delimiter_) {
                state_ = parser_state::end_field;
            } else {
                value_.push_back(chr);
            }
            break;

        case parser_state::in_quoted_field:
            if (chr == '"') {
                state_ = parser_state::quote_in_quoted_field;
            } else {
                value_.push_back(chr);
            }
            break;

        case parser_state::quote_in_quoted_field:
            if (chr == delimiter_) {
                state_ = parser_state::end_field;
            } else if (chr == '"') {
                value_.push_back(chr);
                state_ = parser_state::in_quoted_field;
            } else {
                value_.push_back(chr);
                state_ = parser_state::in_field;
            }
            break;

        case parser_state::finished:
            assert(false);
        }

        ++pos_;

        if (state_ == parser_state::end_field) {
            return true;
        }
    }

    switch (state_) {
    case parser_state::new_field:
    case parser_state::in_field:
    case parser_state::quote_in_quoted_field:
    case parser_state::end_field:
        state_ = parser_state::finished;
        break;

    case parser_state::in_quoted_field:
        throw corrupt_record_error{"EOF reached inside a quoted field."};

    case parser_state::finished:
        assert(false);
    }

    return true;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
