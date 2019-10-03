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

#include "mlio/streams/gzip_inflate_stream.h"

#include <utility>

#include "mlio/streams/detail/zlib.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/stream_error.h"
#include "mlio/util/cast.h"

using mlio::detail::zlib_inflater;

namespace mlio {
inline namespace v1 {

gzip_inflate_stream::
gzip_inflate_stream(intrusive_ptr<input_stream> inner)
    : inner_{std::move(inner)}
{
    inflater_ = std::make_unique<zlib_inflater>();
}

gzip_inflate_stream::~gzip_inflate_stream() = default;

std::size_t
gzip_inflate_stream::
read(mutable_memory_span dest)
{
    check_if_closed();

    if (dest.empty()) {
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
                throw inflate_error{
                    "The zlib stream contains invalid or incomplete deflate "
                    "data."};
            }

            return 0;
        }
    }

    memory_span inp{buffer_pos_, buffer_.end()};

    auto out = dest;

    inflater_->inflate(inp, out);

    buffer_pos_ = buffer_.end() - stdx::ssize(inp);

    return dest.size() - out.size();
}

void
gzip_inflate_stream::
close() noexcept
{
    inner_->close();

    inflater_ = nullptr;

    buffer_ = {};
}

void
gzip_inflate_stream::
check_if_closed() const
{
    if (inner_->closed()) {
        throw stream_error{"The input stream is closed."};
    }
}

bool
gzip_inflate_stream::
closed() const noexcept
{
    return inner_->closed();
}

}  // namespace v1
}  // namespace mlio
