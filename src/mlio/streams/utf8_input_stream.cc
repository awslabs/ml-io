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

#include "mlio/streams/utf8_input_stream.h"

#include <algorithm>
#include <utility>

#include <fmt/format.h>

#include "mlio/logger.h"
#include "mlio/memory/memory_allocator.h"
#include "mlio/streams/detail/iconv.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/stream_error.h"
#include "mlio/util/cast.h"

using mlio::detail::Iconv_desc;
using mlio::detail::Iconv_status;
using mlio::detail::Utf8_input_stream_access;

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

std::size_t fill_buffer(Input_stream &stream, Mutable_memory_span buffer)
{
    Mutable_memory_span remaining = buffer;
    while (!remaining.empty()) {
        std::size_t num_bytes_read = stream.read(remaining);
        if (num_bytes_read == 0) {
            break;
        }

        remaining = remaining.subspan(num_bytes_read);
    }

    return buffer.size() - remaining.size();
}

}  // namespace
}  // namespace detail

Utf8_input_stream::~Utf8_input_stream() = default;

std::size_t Utf8_input_stream::read(Mutable_memory_span destination)
{
    check_if_closed();

    if (destination.empty()) {
        return 0;
    }

    if (remaining_bits_.empty()) {
        // There is no need to convert anything, simply forward the
        // call to the inner stream.
        if (is_utf8_) {
            return inner_->read(destination);
        }

        // The output buffer size must be at least 4-bytes. Otherwise
        // we might end up in an edge case where iconv cannot write
        // anything if the UTF-8 decoded size of the next character is
        // larger than the output size.
        if (destination.size() >= 4) {
            return convert(destination);
        }

        // When the destination argument is smaller than 4-bytes, we
        // convert the character in an internal 4-byte buffer and write
        // to the destination from that buffer.
        std::size_t num_bytes_converted = convert(char_buffer_);
        if (num_bytes_converted == 0) {
            return 0;
        }

        remaining_bits_ = make_span(char_buffer_).first(num_bytes_converted);
    }

    return copy_from_remainder(destination);
}

void Utf8_input_stream::close() noexcept
{
    inner_->close();

    converter_ = nullptr;

    buffer_ = {};
}

bool Utf8_input_stream::closed() const noexcept
{
    return inner_->closed();
}

Utf8_input_stream::Utf8_input_stream(Intrusive_ptr<Input_stream> inner, Text_encoding &&encoding)
    : inner_{std::move(inner)}, is_utf8_{encoding == Text_encoding::utf8}
{
    if (is_utf8_) {
        return;
    }

    converter_ = std::make_unique<Iconv_desc>(std::move(encoding));

    buffer_ = memory_allocator().allocate(0x200'0000);  // 32 MiB

    buffer_pos_ = buffer_->begin();
    buffer_end_ = buffer_->end();
}

std::size_t Utf8_input_stream::convert(Mutable_memory_span destination)
{
    auto out = destination;

    // We use a loop here to make sure that we write at least one byte
    // to the output even if the input buffer contains shift sequences.
    while (destination.size() == out.size()) {
        if (should_fill_buffer_) {
            fill_buffer();
            if (buffer_pos_ == buffer_end_) {
                // We reached the end of the stream; we should check if
                // there are unconsumed bytes in the buffer that cannot
                // be converted.
                if (buffer_pos_ != buffer_->begin()) {
                    throw Stream_error{fmt::format(
                        "An invalid byte sequence encountered while converting from {0} to UTF-8.",
                        converter_->encoding().name())};
                }

                return 0;
            }

            // If there were leftover bytes from the previous iteration
            // or call, we now have to reset the position back to the
            // beginning of the buffer.
            buffer_pos_ = buffer_->begin();

            should_fill_buffer_ = false;
        }

        Memory_span inp{buffer_pos_, buffer_end_};

        Iconv_status s = converter_->convert(inp, out);

        // If the buffer ends with a partial multi-byte character, we
        // have to move the leftover bits to the beginning of the
        // buffer and refill the rest.
        if (s == Iconv_status::incomplete_char) {
            buffer_pos_ = std::copy(inp.begin(), inp.end(), buffer_->begin());

            should_fill_buffer_ = true;

            continue;
        }

        // As the smallest output buffer size that we provide to the
        // decode() function is 4-bytes, we are guaranteed to have at
        // least one UTF-8 character written to the output.
        if (s == Iconv_status::leftover) {
            buffer_pos_ = buffer_end_ - stdx::ssize(inp);

            continue;
        }

        buffer_pos_ = buffer_->begin();

        should_fill_buffer_ = true;
    }

    return destination.size() - out.size();
}

void Utf8_input_stream::fill_buffer()
{
    Mutable_memory_span buffer{buffer_pos_, buffer_end_};

    std::size_t num_bytes_read = detail::fill_buffer(*inner_, buffer);

    buffer_end_ = buffer_pos_ + as_ssize(num_bytes_read);
}

std::size_t Utf8_input_stream::copy_from_remainder(Mutable_memory_span destination) noexcept
{
    std::size_t size = std::min(remaining_bits_.size(), destination.size());

    std::copy_n(remaining_bits_.begin(), size, destination.begin());

    remaining_bits_ = remaining_bits_.subspan(size);

    return size;
}

void Utf8_input_stream::set_preamble(Memory_span value) noexcept
{
    // If the underlying stream is non-seekable, we first need to serve
    // the preamble bytes that we read during instantiation.
    if (is_utf8_) {
        std::copy(value.begin(), value.end(), char_buffer_.begin());

        // The stream is UTF-8; treat the preamble bytes as if they were
        // already converted.
        remaining_bits_ = make_span(char_buffer_).first(value.size());
    }
    else {
        // Otherwise put them into the buffer so they get converted by
        // the next call to read().
        buffer_pos_ = std::copy(value.begin(), value.end(), buffer_->begin());
    }
}

void Utf8_input_stream::check_if_closed() const
{
    if (inner_->closed()) {
        throw Stream_error{"The input stream is closed."};
    }
}

namespace detail {

struct Utf8_input_stream_access {
    template<typename... Args>
    static inline Intrusive_ptr<Utf8_input_stream> make(Args &&... args)
    {
        auto *ptr = new Utf8_input_stream{std::forward<Args>(args)...};

        return wrap_intrusive(ptr);
    }

    static inline void set_preamble(Utf8_input_stream &stream, Memory_span value)
    {
        stream.set_preamble(value);
    }
};

namespace {

std::optional<Text_encoding> infer_bom_encoding(Memory_span preamble) noexcept
{
    auto chars = as_span<const unsigned char>(preamble);

    if (chars.size() >= 3) {
        if (chars[0] == 0xEF && chars[1] == 0xBB && chars[2] == 0xBF) {
            return Text_encoding::utf8;
        }
    }

    if (chars.size() == 4) {
        if (chars[0] == 0x00 && chars[1] == 0x00 && chars[2] == 0xFE && chars[3] == 0xFF) {
            return Text_encoding::utf32_be;
        }

        if (chars[0] == 0xFF && chars[1] == 0xFE && chars[2] == 0x00 && chars[3] == 0x00) {
            return Text_encoding::utf32_le;
        }
    }

    if (chars.size() >= 2) {
        if (chars[0] == 0xFE && chars[1] == 0xFF) {
            return Text_encoding::utf16_be;
        }

        if (chars[0] == 0xFF && chars[1] == 0xFE) {
            return Text_encoding::utf16_le;
        }
    }

    return {};
}

}  // namespace
}  // namespace detail

Intrusive_ptr<Input_stream>
make_utf8_stream(Intrusive_ptr<Input_stream> stream, std::optional<Text_encoding> encoding)
{
    if (encoding != std::nullopt) {
        if (*encoding == Text_encoding::utf8 || *encoding == Text_encoding::ascii_latin1) {
            return stream;
        }
    }

    // The length of a Unicode BOM can be at most 4-bytes.
    std::array<std::byte, 4> buffer{};

    Mutable_memory_span preamble{};
    if (encoding == std::nullopt) {
        std::size_t num_bytes_read = detail::fill_buffer(*stream, buffer);

        preamble = make_span(buffer).first(num_bytes_read);

        // Try to infer the encoding from the stream. If we cannot find
        // a BOM in the preamble, we assume the stream is UTF-8.
        encoding = detail::infer_bom_encoding(preamble);
        if (encoding == std::nullopt) {
            // If the stream is seekable, take the shortcut.
            if (stream->seekable()) {
                stream->seek(0);

                return stream;
            }

            encoding = Text_encoding::utf8;
        }
        else {
            logger::debug("The stream starts with a {0} BOM.", encoding->name());
        }
    }

    auto wrapped_stream = Utf8_input_stream_access::make(stream, std::move(*encoding));

    if (stream->seekable()) {
        stream->seek(0);
    }
    else {
        // If the stream is non-seekable (e.g. SageMaker pipe) we need
        // to make sure that we do not lose the preamble.
        Utf8_input_stream_access::set_preamble(*wrapped_stream, preamble);
    }

    return std::move(wrapped_stream);
}

}  // namespace abi_v1
}  // namespace mlio
