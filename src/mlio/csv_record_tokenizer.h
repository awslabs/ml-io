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

#pragma once

#include <string>

#include "mlio/span.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class csv_record_tokenizer {
private:
    enum class parser_state {
        new_field,
        in_field,
        in_quoted_field,
        quote_in_quoted_field,
        end_field,
        finished
    };

public:
    explicit
    csv_record_tokenizer(memory_span blob, char delimiter) noexcept
        : text_{as_span<char const>(blob)}, delimiter_{delimiter}
    {}

public:
    bool
    next();

    std::string const &
    value() const noexcept
    {
        return value_;
    }

    bool
    eof() const noexcept
    {
        return eof_;
    }

private:
    stdx::span<char const> text_;
    stdx::span<char const>::iterator pos_ = text_.begin();
    std::string value_;
    char delimiter_;
    parser_state state_ = parser_state::new_field;
    bool eof_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
