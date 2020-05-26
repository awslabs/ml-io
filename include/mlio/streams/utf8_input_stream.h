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

#include <array>
#include <cstddef>
#include <memory>
#include <optional>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream_base.h"
#include "mlio/text_encoding.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

struct Utf8_input_stream_access;

}  // namespace detail

/// @addtogroup streams Streams
/// @{

/// Represents an @ref Input_stream that converts the bits read from
/// the underlying stream to UTF-8.
class MLIO_API Utf8_input_stream final : public Input_stream_base {
    friend struct detail::Utf8_input_stream_access;

public:
    Utf8_input_stream(const Utf8_input_stream &) = delete;

    Utf8_input_stream &operator=(const Utf8_input_stream &) = delete;

    Utf8_input_stream(Utf8_input_stream &&) = delete;

    Utf8_input_stream &operator=(Utf8_input_stream &&) = delete;

    ~Utf8_input_stream() final;

    using Input_stream_base::read;

    std::size_t read(Mutable_memory_span destination) final;

    void close() noexcept final;

    bool closed() const noexcept final;

private:
    explicit Utf8_input_stream(Intrusive_ptr<Input_stream> inner, Text_encoding &&encoding);

    MLIO_HIDDEN
    std::size_t convert(Mutable_memory_span destination);

    MLIO_HIDDEN
    void fill_buffer();

    MLIO_HIDDEN
    std::size_t copy_from_remainder(Mutable_memory_span destination) noexcept;

    MLIO_HIDDEN
    void set_preamble(Memory_span value) noexcept;

    MLIO_HIDDEN
    void check_if_closed() const;

    Intrusive_ptr<Input_stream> inner_;
    bool is_utf8_;
    std::unique_ptr<detail::Iconv_desc> converter_;
    Intrusive_ptr<Mutable_memory_block> buffer_{};
    Mutable_memory_block::iterator buffer_pos_{};
    Mutable_memory_block::iterator buffer_end_{};
    bool should_fill_buffer_ = true;
    std::array<std::byte, 4> char_buffer_{};
    Memory_span remaining_bits_{};
};

/// Constructs a new @ref Utf8_input_stream from the specified @ref
/// Input_stream. If no encoding is specified, the encoding is inferred
/// from the preamble of the underlying stream. If no preamble is found,
/// falls back to UTF-8.
MLIO_API
Intrusive_ptr<Input_stream>
make_utf8_stream(Intrusive_ptr<Input_stream> stream, std::optional<Text_encoding> encoding = {});

/// @}

}  // namespace abi_v1
}  // namespace mlio
