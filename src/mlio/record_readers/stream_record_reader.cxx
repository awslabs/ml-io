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

#include "mlio/record_readers/stream_record_reader.h"

#include <utility>

#include "mlio/optional.h"
#include "mlio/record_readers/detail/chunk_reader.h"
#include "mlio/record_readers/record.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {

stream_record_reader::
stream_record_reader(intrusive_ptr<input_stream> strm)
{
    chunk_reader_ = detail::make_chunk_reader(std::move(strm));
}

stream_record_reader::~stream_record_reader() = default;

stdx::optional<record>
stream_record_reader::
read_record_core()
{
    stdx::optional<record> rec{};

    bool ignore_leftover = !chunk_reader_->eof();

    while ((rec = decode_record(chunk_, ignore_leftover)) == stdx::nullopt) {
        chunk_ = chunk_reader_->read_chunk(chunk_);
        if (chunk_.empty()) {
            return {};
        }
    }

    return rec;
}

std::size_t
stream_record_reader::
record_size_hint() const noexcept
{
    return chunk_reader_->chunk_size_hint();
}

void
stream_record_reader::
set_record_size_hint(std::size_t value) noexcept
{
    chunk_reader_->set_chunk_size_hint(value);
}

}  // namespace v1
}  // namespace mlio
