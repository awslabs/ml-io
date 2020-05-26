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
inline namespace abi_v1 {
namespace detail {

class Csv_record_reader final : public Text_record_reader {
public:
    explicit Csv_record_reader(Intrusive_ptr<Input_stream> stream, const Csv_params &params)
        : Text_record_reader{std::move(stream)}, params_{&params}
    {}

private:
    enum class Parser_state {
        new_field,
        in_field,
        in_quoted_field,
        quote_in_quoted_field,
        has_carriage
    };

    std::optional<Record> decode_text_record(Memory_slice &chunk, bool ignore_leftover) final;

    bool is_comment_line(const Memory_slice &chunk);

    std::optional<Record> read_line(Memory_slice &chunk, bool ignore_leftover);

    static bool try_get_next_char(const stdx::span<const char> &chars,
                                  stdx::span<const char>::iterator &pos,
                                  char &chr) noexcept;

    static void check_line_length(const stdx::span<const char> &chars,
                                  stdx::span<const char>::iterator &pos,
                                  std::size_t max_line_length);

    const Csv_params *params_;
};

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
