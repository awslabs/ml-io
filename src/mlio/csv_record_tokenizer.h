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

#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "mlio/csv_reader.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class csv_record_tokenizer {
private:
    enum class parser_state { new_field, in_field, in_quoted_field, quote_in_quoted_field };

public:
    explicit csv_record_tokenizer(const csv_params &prm) noexcept : csv_record_tokenizer{prm, {}}
    {}

    explicit csv_record_tokenizer(const csv_params &prm, memory_span blob) noexcept
        : text_{as_span<char const>(blob)}
        , delimiter_{prm.delimiter}
        , quote_char_{prm.quote_char}
        , max_field_length_{prm.max_field_length}
    {}

public:
    bool next();

    void reset(memory_span blob);

private:
    bool try_get_next_char(char &chr) noexcept;

    void push_char(char chr) noexcept;

public:
    const std::string &value() const noexcept
    {
        return value_;
    }

    bool is_truncated() const noexcept
    {
        return is_truncated_;
    }

    bool eof() const noexcept
    {
        return eof_;
    }

private:
    stdx::span<char const> text_{};
    stdx::span<char const>::iterator text_pos_ = text_.begin();
    char delimiter_;
    char quote_char_;
    std::optional<std::size_t> max_field_length_{};
    std::string value_{};
    bool is_truncated_{};
    bool is_finished_{};
    bool eof_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
