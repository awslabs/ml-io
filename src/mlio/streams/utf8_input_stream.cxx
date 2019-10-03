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

#include "mlio/streams/utf8_input_stream.h"

#include <algorithm>
#include <utility>

#include <fmt/format.h>

#include "mlio/logger.h"
#include "mlio/memory/memory_allocator.h"
#include "mlio/streams/detail/iconv.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/stream_error.h"
#include "mlio/text_encoding.h"
#include "mlio/util/cast.h"

using mlio::detail::iconv_desc;
using mlio::detail::iconv_status;
using mlio::detail::utf8_input_stream_access;

namespace mlio {
inline namespace v1 {
namespace detail {

struct utf8_input_stream_access {
    template<typename... Args>
    static inline auto
    make(Args &&...args)
    {
        auto *ptr = new utf8_input_stream{std::forward<Args>(args)...};

        return wrap_intrusive(ptr);
    }

    static inline void
    set_preamble(utf8_input_stream &strm, memory_span value)
    {
        strm.set_preamble(value);
    }
};

namespace {

std::size_t
fill_buffer(input_stream &strm, mutable_memory_span buffer)
{
    mutable_memory_span remaining = buffer;
    while (!remaining.empty()) {
        std::size_t num_bytes_read = strm.read(remaining);
        if (num_bytes_read == 0) {
            break;
        }

        remaining = remaining.subspan(num_bytes_read);
    }

    return buffer.size() - remaining.size();
}

stdx::optional<text_encoding>
infer_bom_encoding(memory_span preamble) noexcept
{
    auto chrs = as_span<unsigned char const>(preamble);

    if (chrs.size() >= 3) {
        if (chrs[0] == 0xEF && chrs[1] == 0xBB && chrs[2] == 0xBF) {
            return text_encoding::utf8;
        }
    }

    if (chrs.size() == 4) {
        if (chrs[0] == 0x00 && chrs[1] == 0x00 &&
            chrs[2] == 0xFE && chrs[3] == 0xFF) {

            return text_encoding::utf32_be;
        }

        if (chrs[0] == 0xFF && chrs[1] == 0xFE &&
            chrs[2] == 0x00 && chrs[3] == 0x00) {

            return text_encoding::utf32_le;
        }
    }

    if (chrs.size() >= 2) {
        if (chrs[0] == 0xFE && chrs[1] == 0xFF) {
            return text_encoding::utf16_be;
        }

        if (chrs[0] == 0xFF && chrs[1] == 0xFE) {
            return text_encoding::utf16_le;
        }
    }

    return {};
}

}  // namespace
}  // namespace detail

intrusive_ptr<input_stream>
make_utf8_stream(intrusive_ptr<input_stream> strm, stdx::optional<text_encoding> enc)
{
    if (enc != stdx::nullopt) {
        if (*enc == text_encoding::utf8 ||
            *enc == text_encoding::ascii_latin1) {

            return strm;
        }
    }

    // The length of a Unicode BOM can be at most 4-bytes.
    std::array<stdx::byte, 4> buffer{};

    mutable_memory_span preamble{};
    if (enc == stdx::nullopt) {
        std::size_t num_bytes_read = detail::fill_buffer(*strm, buffer);

        preamble = make_span(buffer).first(num_bytes_read);

        // Try to infer the encoding from the stream. If we cannot find
        // a BOM in the preamble, we assume the stream is UTF-8.
        enc = detail::infer_bom_encoding(preamble);
        if (enc == stdx::nullopt) {
            // If the stream is seekable, take the shortcut.
            if (strm->seekable()) {
                strm->seek(0);

                return strm;
            }

            enc = text_encoding::utf8;
        } else {
            logger::debug("The stream starts with a {0} BOM.", enc->name());
        }
    }

    auto wrap = utf8_input_stream_access::make(strm, std::move(*enc));

    if (strm->seekable()) {
        strm->seek(0);
    } else {
        // If the stream is non-seekable (e.g. SageMaker pipe) we need
        // to make sure that we do not lose the preamble.
        utf8_input_stream_access::set_preamble(*wrap, preamble);
    }

    return std::move(wrap);
}

utf8_input_stream::
utf8_input_stream(intrusive_ptr<input_stream> inner, text_encoding &&enc)
    : inner_{std::move(inner)}, is_utf8_{enc == text_encoding::utf8}
{
    if (is_utf8_) {
        return;
    }

    converter_ = std::make_unique<iconv_desc>(std::move(enc));

    buffer_ = get_memory_allocator().allocate(0x200'0000);  // 32 MiB

    buffer_pos_ = buffer_->begin();
    buffer_end_ = buffer_->end();
}

utf8_input_stream::~utf8_input_stream() = default;

std::size_t
utf8_input_stream::
read(mutable_memory_span dest)
{
    check_if_closed();

    if (dest.empty()) {
        return 0;
    }

    if (remaining_bits_.empty()) {
        // There is no need to convert anything, simply forward the
        // call to the inner stream.
        if (is_utf8_) {
            return inner_->read(dest);
        }

        // The output buffer size must be at least 4-bytes. Otherwise
        // we might end up in an edge case where iconv cannot write
        // anything if the UTF-8 decoded size of the next character is
        // larger than the output size.
        if (dest.size() >= 4) {
            return convert(dest);
        }

        // When the dest argument is smaller than 4-bytes, we convert
        // the character in an internal 4-byte buffer and write to dest
        // from that buffer.
        std::size_t num_bytes_conv = convert(char_buffer_);
        if (num_bytes_conv == 0) {
            return 0;
        }

        remaining_bits_ = make_span(char_buffer_).first(num_bytes_conv);
    }

    return copy_from_remainder(dest);
}

std::size_t
utf8_input_stream::
convert(mutable_memory_span dest)
{
    auto out = dest;

    // We use a loop here to make sure that we write at least one byte
    // to the output even if the input buffer contains shift sequences.
    while (dest.size() == out.size()) {
        if (should_fill_buffer_) {
            fill_buffer();
            if (buffer_pos_ == buffer_end_) {
                // We reached the end of the stream; we should check if
                // there are unconsumed bytes in the buffer that cannot
                // be converted.
                if (buffer_pos_ != buffer_->begin()) {
                    throw stream_error{fmt::format(
                        "An invalid byte sequence encountered while "
                        "converting from {0} to UTF-8.",
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

        memory_span inp{buffer_pos_, buffer_end_};

        iconv_status s = converter_->convert(inp, out);

        // If the buffer ends with a partial multi-byte character, we
        // have to move the leftover bits to the beginning of the
        // buffer and refill the rest.
        if (s == iconv_status::incomplete_char) {
            buffer_pos_ = std::copy(inp.begin(), inp.end(), buffer_->begin());

            should_fill_buffer_ = true;

            continue;
        }

        // As the smallest output buffer size that we provide to the
        // decode() function is 4-bytes, we are guaranteed to have at
        // least one UTF-8 character written to the output.
        if (s == iconv_status::leftover) {
            buffer_pos_ = buffer_end_ - stdx::ssize(inp);

            continue;
        }

        buffer_pos_ = buffer_->begin();

        should_fill_buffer_ = true;
    }

    return dest.size() - out.size();
}

void
utf8_input_stream::
fill_buffer()
{
    mutable_memory_span buffer{buffer_pos_, buffer_end_};

    std::size_t num_bytes_read = detail::fill_buffer(*inner_, buffer);

    buffer_end_ = buffer_pos_ + as_ssize(num_bytes_read);
}

std::size_t
utf8_input_stream::
copy_from_remainder(mutable_memory_span dest) noexcept
{
    std::size_t size = std::min(remaining_bits_.size(), dest.size());

    std::copy_n(remaining_bits_.begin(), size, dest.begin());

    remaining_bits_ = remaining_bits_.subspan(size);

    return size;
}

void
utf8_input_stream::
set_preamble(memory_span value) noexcept
{
    // If the underlying stream is non-seekable, we first need to serve
    // the preamble bytes that we read during instantiation.
    if (is_utf8_) {
        std::copy(value.begin(), value.end(), char_buffer_.begin());

        // The stream is UTF-8; treat the preamble bytes as if they were
        // already converted.
        remaining_bits_ = make_span(char_buffer_).first(value.size());
    } else {
        // Otherwise put them into the buffer so they get converted by
        // the next call to read().
        buffer_pos_ = std::copy(value.begin(), value.end(), buffer_->begin());
    }
}

void
utf8_input_stream::
close() noexcept
{
    inner_->close();

    converter_ = nullptr;

    buffer_ = {};
}

void
utf8_input_stream::
check_if_closed() const
{
    if (inner_->closed()) {
        throw stream_error{"The input stream is closed."};
    }
}

bool
utf8_input_stream::
closed() const noexcept
{
    return inner_->closed();
}

}  // namespace v1
}  // namespace mlio
