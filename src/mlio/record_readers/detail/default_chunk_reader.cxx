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

#include "mlio/record_readers/detail/default_chunk_reader.h"

#include <algorithm>

#include "mlio/memory/memory_allocator.h"
#include "mlio/memory/util.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {

memory_slice
default_chunk_reader::
read_chunk(memory_span leftover)
{
    if (eof_) {
        return {};
    }

    bool reuse_buffer = false;

    if (chunk_ != nullptr) {
        // If the whole chunk is leftover, it means it does not contain
        // any records; in such case we should increase the size of the
        // chunk to make sure that we fit at least one record into it.
        if (chunk_->size() == leftover.size()) {
            if (chunk_->size() == next_chunk_size_) {
                next_chunk_size_ <<= 1;
            }

            reuse_buffer = true;

        // If the chunk is owned only by this chunk_reader and its
        // associated stream_record_reader, we can safely re-use it for
        // the next fill operation.
        } else if (chunk_->use_count() <= 2) {
            if (!leftover.empty()) {
                std::copy(leftover.begin(), leftover.end(), chunk_->begin());
            }

            reuse_buffer = true;
        }
    }

    if (reuse_buffer) {
        if (chunk_->size() != next_chunk_size_) {
            chunk_ = resize_memory_block(chunk_, next_chunk_size_);
        }
    } else {
        chunk_ = get_memory_allocator().allocate(next_chunk_size_);

        if (!leftover.empty()) {
            std::copy(leftover.begin(), leftover.end(), chunk_->begin());
        }
    }

    auto remaining = make_span(*chunk_).subspan(leftover.size());
    while (!remaining.empty()) {
        std::size_t num_bytes_read = stream_->read(remaining);
        if (num_bytes_read == 0) {
            eof_ = true;

            break;
        }

        remaining = remaining.subspan(num_bytes_read);
    }

    intrusive_ptr<mutable_memory_block> chunk;
    if (eof_) {
        chunk = std::move(chunk_);
    } else {
        chunk = chunk_;
    }

    return memory_slice{chunk}
        .first(chunk->end() - stdx::ssize(remaining));
}

void
default_chunk_reader::
set_chunk_size_hint(std::size_t value) noexcept
{
    while (value > next_chunk_size_) {
        next_chunk_size_ <<= 1;
    }
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
