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

#include "mlio/streams/memory_input_stream.h"

#include <algorithm>

#include "mlio/byte.h"
#include "mlio/streams/stream_error.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {

std::size_t
memory_input_stream::
read(mutable_memory_span dest)
{
    check_if_closed();

    if (dest.empty()) {
        return 0;
    }

    auto old_pos = pos_;

    advance_position(dest.size());

    auto last = std::copy(old_pos, pos_, dest.begin());

    return as_size(last - dest.begin());
}

memory_slice
memory_input_stream::
read(std::size_t size)
{
    check_if_closed();

    if (size == 0) {
        return {};
    }

    auto old_pos = pos_;

    advance_position(size);

    return source_.subslice(old_pos, pos_);
}

void
memory_input_stream::
seek(std::size_t position)
{
    check_if_closed();

    pos_ = source_.begin();

    advance_position(position);
}

void
memory_input_stream::
close() noexcept
{
    source_ = {};

    closed_ = true;
}

void
memory_input_stream::
advance_position(std::size_t dist) noexcept
{
    pos_ = std::min(pos_ + as_ssize(dist), source_.end());
}

void
memory_input_stream::
check_if_closed() const
{
    if (closed_) {
        throw stream_error{"The input stream is closed."};
    }
}

std::size_t
memory_input_stream::
size() const
{
    check_if_closed();

    return source_.size();
}

std::size_t
memory_input_stream::
position() const
{
    check_if_closed();

    return as_size(pos_ - source_.begin());
}

}  // namespace v1
}  // namespace mlio
