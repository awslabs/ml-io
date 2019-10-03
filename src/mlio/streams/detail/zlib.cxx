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

#include "mlio/streams/detail/zlib.h"

#include <cassert>
#include <new>

#include "mlio/not_supported_error.h"
#include "mlio/streams/stream_error.h"

namespace mlio {
inline namespace v1 {
namespace detail {

zlib_inflater::
zlib_inflater()
{
    // Inflate both zlib and gzip.
    int r = ::inflateInit2(&stream_, MAX_WBITS + 32);
    if (r == Z_OK) {
        return;
    }

    if (r == Z_MEM_ERROR) {
        throw std::bad_alloc{};
    }
    if (r == Z_VERSION_ERROR) {
        throw not_supported_error{
            "The zlib library has an unsupported version."};
    }
    assert(false);
}

zlib_inflater::~zlib_inflater()
{
    ::inflateEnd(&stream_);
}

void
zlib_inflater::
inflate(memory_span &inp, mutable_memory_span &out)
{
    validate_state();

    auto i_buf = as_span<::Bytef const>(inp);
    auto o_buf = as_span<::Bytef>(out);

    // We do not use the ZLIB_CONST macro because some older
    // distributions we have to support do not have an up-to-date libz.
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    stream_.next_in  = const_cast<::Bytef *>(i_buf.data());
    stream_.next_out = o_buf.data();

    stream_.avail_in  = static_cast<::uInt>(i_buf.size());
    stream_.avail_out = static_cast<::uInt>(o_buf.size());

    state_ = ::inflate(&stream_, Z_NO_FLUSH);

    validate_state();

    if (state_ == Z_STREAM_END) {
        ::inflateReset(&stream_);
    }

    inp = inp.last(stream_.avail_in);
    out = out.last(stream_.avail_out);
}

void
zlib_inflater::
validate_state()
{
    switch (state_) {
    case Z_OK:
    case Z_STREAM_END:
        return;

    case Z_NEED_DICT:
    case Z_DATA_ERROR:
        throw inflate_error{
            "The zlib stream contains invalid or incomplete deflate data."};

    case Z_MEM_ERROR:
        throw std::bad_alloc{};
    }

    assert(false);
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
