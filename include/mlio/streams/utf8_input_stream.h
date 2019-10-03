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

#include <array>
#include <cstddef>
#include <memory>

#include "mlio/byte.h"
#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/optional.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream_base.h"

namespace mlio {
inline namespace v1 {
namespace detail {

struct utf8_input_stream_access;

}  // namespace detail

/// @addtogroup streams Streams
/// @{

/// Represents an @ref input_stream that converts the bits read from
/// the underlying stream to UTF-8.
class MLIO_API utf8_input_stream final : public input_stream_base {
    friend struct detail::utf8_input_stream_access;

private:
    explicit
    utf8_input_stream(intrusive_ptr<input_stream> inner, text_encoding &&enc);

public:
    utf8_input_stream(utf8_input_stream const &) = delete;

    utf8_input_stream(utf8_input_stream &&) = delete;

   ~utf8_input_stream() final;

public:
    utf8_input_stream &
    operator=(utf8_input_stream const &) = delete;

    utf8_input_stream &
    operator=(utf8_input_stream &&) = delete;

public:
    using input_stream_base::read;

    std::size_t
    read(mutable_memory_span dest) final;

    void
    close() noexcept final;

private:
    MLIO_HIDDEN
    std::size_t
    convert(mutable_memory_span dest);

    MLIO_HIDDEN
    void
    fill_buffer();

    MLIO_HIDDEN
    std::size_t
    copy_from_remainder(mutable_memory_span dest) noexcept;

    MLIO_HIDDEN
    void
    set_preamble(memory_span value) noexcept;

    MLIO_HIDDEN
    void
    check_if_closed() const;

public:
    bool
    closed() const noexcept final;

private:
    intrusive_ptr<input_stream> inner_;
    bool is_utf8_;
    std::unique_ptr<detail::iconv_desc> converter_;
    intrusive_ptr<mutable_memory_block> buffer_{};
    mutable_memory_block::iterator buffer_pos_{};
    mutable_memory_block::iterator buffer_end_{};
    bool should_fill_buffer_ = true;
    std::array<stdx::byte, 4> char_buffer_{};
    memory_span remaining_bits_{};
};

/// Constructs a new @ref utf8_input_stream from the specified @ref
/// input_stream. If no encoding is specified, the encoding is inferred
/// from the preamble of the underlying stream. If no preamble is found,
/// falls back to UTF-8.
MLIO_API
intrusive_ptr<input_stream>
make_utf8_stream(intrusive_ptr<input_stream> strm, stdx::optional<text_encoding> enc);

/// @}

}  // namespace v1
}  // namespace mlio
