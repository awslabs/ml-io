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

#include "mlio/streams/detail/iconv.h"

#include <cerrno>
#include <cstddef>
#include <system_error>
#include <utility>

#include <fmt/format.h>

#include "mlio/detail/error.h"
#include "mlio/streams/stream_error.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Iconv_desc::Iconv_desc(Text_encoding &&encoding) : encoding_{std::move(encoding)}
{
    auto code = encoding_.name().c_str();

    desc_ = ::iconv_open(Text_encoding::utf8.name().c_str(), code);
    if (desc_ != reinterpret_cast<::iconv_t>(-1)) {
        return;
    }

    std::error_code err = current_error_code();
    if (err == std::errc::invalid_argument) {
        throw std::system_error{
            err,
            fmt::format("The {0} encoding is not supported by the platform.", encoding_.name())};
    }
    throw std::system_error{
        err,
        fmt::format("An unexpected error occurred while initializing the {0} converter.",
                    encoding_.name())};
}

Iconv_desc::~Iconv_desc()
{
    ::iconv_close(desc_);
}

Iconv_status Iconv_desc::convert(Memory_span &inp, Mutable_memory_span &out)
{
    auto i_chars = as_span<const char>(inp);
    auto o_chars = as_span<char>(out);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto i_data = const_cast<char *>(i_chars.data());
    auto i_left = i_chars.size();

    auto o_data = o_chars.data();
    auto o_left = o_chars.size();

    std::size_t r = ::iconv(desc_, &i_data, &i_left, &o_data, &o_left);

    inp = inp.last(i_left);
    out = out.last(o_left);

    if (static_cast<std::ptrdiff_t>(r) != -1) {
        return Iconv_status::ok;
    }

    if (errno == EINVAL) {
        return Iconv_status::incomplete_char;
    }
    if (errno == E2BIG) {
        return Iconv_status::leftover;
    }
    if (errno == EILSEQ) {
        throw Stream_error{
            fmt::format("An invalid byte sequence encountered while converting from {0} to UTF-8.",
                        encoding_.name())};
    }

    std::error_code err = current_error_code();
    throw std::system_error{
        err,
        fmt::format(
            "An unexpected error occurred while trying to convert a byte sequence from {0} to UTF-8.",
            encoding_.name())};
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
