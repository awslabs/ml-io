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

#include <optional>
#include <utility>

#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/record_readers/text_record_reader.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class csv_record_reader final : public text_record_reader {
    enum class parser_state {
        new_field,
        in_field,
        in_quoted_field,
        quote_in_quoted_field,
        has_carriage
    };

public:
    explicit csv_record_reader(intrusive_ptr<input_stream> strm, csv_params const &prm)
        : text_record_reader{std::move(strm)}, params_{&prm}
    {}

private:
    std::optional<record> decode_text_record(memory_slice &chunk, bool ignore_leftover) final;

    bool is_comment_line(memory_slice const &chunk);

    std::optional<record> read_line(memory_slice &chunk, bool ignore_leftover);

    static bool try_get_next_char(stdx::span<char const> const &chrs,
                                  stdx::span<char const>::iterator &pos,
                                  char &chr) noexcept;

    static void check_line_length(stdx::span<char const> const &chrs,
                                  stdx::span<char const>::iterator &pos,
                                  std::size_t max_line_length);

private:
    csv_params const *params_;
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
