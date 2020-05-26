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

#include "mlio/streams/gzip_inflate_stream.h"

#include <utility>

#include "mlio/streams/detail/zlib.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/stream_error.h"
#include "mlio/util/cast.h"

using mlio::detail::Zlib_inflater;

namespace mlio {
inline namespace abi_v1 {

Gzip_inflate_stream::Gzip_inflate_stream(Intrusive_ptr<Input_stream> inner)
    : inner_{std::move(inner)}
{
    inflater_ = std::make_unique<Zlib_inflater>();
}

Gzip_inflate_stream::~Gzip_inflate_stream() = default;

std::size_t Gzip_inflate_stream::read(Mutable_memory_span destination)
{
    check_if_closed();

    if (destination.empty()) {
        return 0;
    }

    if (buffer_pos_ == buffer_.end()) {
        buffer_ = inner_->read(0x8'0000);  // 512 KiB

        // Make sure to reset the position before checking whether we
        // reached the end of the stream; otherwise the function won't
        // behave correctly if called a second time.
        buffer_pos_ = buffer_.begin();

        if (buffer_.empty()) {
            if (!inflater_->eof()) {
                throw Inflate_error{"The zlib stream contains invalid or incomplete deflate data."};
            }

            return 0;
        }
    }

    Memory_span inp{buffer_pos_, buffer_.end()};

    auto out = destination;

    inflater_->inflate(inp, out);

    buffer_pos_ = buffer_.end() - stdx::ssize(inp);

    return destination.size() - out.size();
}

void Gzip_inflate_stream::close() noexcept
{
    inner_->close();

    inflater_ = nullptr;

    buffer_ = {};
}

bool Gzip_inflate_stream::closed() const noexcept
{
    return inner_->closed();
}

void Gzip_inflate_stream::check_if_closed() const
{
    if (inner_->closed()) {
        throw Stream_error{"The input stream is closed."};
    }
}

}  // namespace abi_v1
}  // namespace mlio
