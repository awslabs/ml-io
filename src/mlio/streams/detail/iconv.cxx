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

#include "mlio/streams/detail/iconv.h"

#include <cerrno>
#include <cstddef>
#include <string>
#include <system_error>
#include <utility>

#include <fmt/format.h>

#include "mlio/detail/error.h"
#include "mlio/streams/stream_error.h"

namespace mlio {
inline namespace v1 {
namespace detail {

iconv_desc::
iconv_desc(text_encoding &&enc)
    : encoding_{std::move(enc)}
{
    auto code = encoding_.name().c_str();

    desc_ = ::iconv_open(text_encoding::utf8.name().c_str(), code);
    if (desc_ != reinterpret_cast<::iconv_t>(-1)) {
        return;
    }

    std::error_code err = current_error_code();
    if (err == std::errc::invalid_argument) {
        throw std::system_error{err, fmt::format(
            "The {0} encoding is not supported by the platform.",
            encoding_.name())};
    }
    throw std::system_error{err, fmt::format(
        "An unexpected error occured while initializing the {0} converter.",
        encoding_.name())};
}

iconv_desc::~iconv_desc()
{
    ::iconv_close(desc_);
}

iconv_status
iconv_desc::
convert(memory_span &inp, mutable_memory_span &out)
{
    auto i_chrs = as_span<char const>(inp);
    auto o_chrs = as_span<char>(out);

    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    auto i_data = const_cast<char *>(i_chrs.data());
    auto i_left = i_chrs.size();

    auto o_data = o_chrs.data();
    auto o_left = o_chrs.size();

    std::size_t r = ::iconv(desc_, &i_data, &i_left, &o_data, &o_left);

    inp = inp.last(i_left);
    out = out.last(o_left);

    if (static_cast<std::ptrdiff_t>(r) != -1) {
        return iconv_status::ok;
    }

    if (errno == EINVAL) {
        return iconv_status::incomplete_char;
    }
    if (errno == E2BIG) {
        return iconv_status::leftover;
    }
    if (errno == EILSEQ) {
        throw stream_error{fmt::format(
            "An invalid byte sequence encountered while converting from {0} "
            "to UTF-8.",
            encoding_.name())};
    }

    std::error_code err = current_error_code();
    throw std::system_error{err, fmt::format(
        "An unexpected error occured while trying to convert a byte sequence "
        "from {0} to UTF-8.",
        encoding_.name())};
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
