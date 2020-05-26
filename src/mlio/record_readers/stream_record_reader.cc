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

#include "mlio/record_readers/stream_record_reader.h"

#include <optional>
#include <utility>

#include "mlio/record_readers/detail/chunk_reader.h"
#include "mlio/record_readers/record.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace abi_v1 {

Stream_record_reader::~Stream_record_reader() = default;

std::size_t Stream_record_reader::record_size_hint() const noexcept
{
    return chunk_reader_->chunk_size_hint();
}

void Stream_record_reader::set_record_size_hint(std::size_t value) noexcept
{
    chunk_reader_->set_chunk_size_hint(value);
}

Stream_record_reader::Stream_record_reader(Intrusive_ptr<Input_stream> stream)
{
    chunk_reader_ = detail::make_chunk_reader(std::move(stream));
}

std::optional<Record> Stream_record_reader::read_record_core()
{
    std::optional<Record> record{};

    while (true) {
        record = decode_record(chunk_, !chunk_reader_->eof());
        if (record) {
            break;
        }

        chunk_ = chunk_reader_->read_chunk(chunk_);
        if (chunk_.empty()) {
            break;
        }
    }

    return record;
}

}  // namespace abi_v1
}  // namespace mlio
