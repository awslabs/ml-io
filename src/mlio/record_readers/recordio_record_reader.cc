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

#include "mlio/record_readers/recordio_record_reader.h"

#include <cstddef>

#include <fmt/format.h>

#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/detail/recordio_header.h"
#include "mlio/record_readers/detail/util.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_error.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

std::optional<Record>
Recordio_record_reader::decode_record(Memory_slice &chunk, bool ignore_leftover)
{
    if (chunk.empty()) {
        return {};
    }

    auto header = detail::decode_recordio_header(chunk);
    if (header == std::nullopt) {
        if (ignore_leftover) {
            return {};
        }

        throw Corrupt_header_error{"The record does not have a valid RecordIO header."};
    }

    std::size_t payload_size = header->payload_size();

    // The MXNet RecordIO format requires records to be on 4-byte
    // boundary.
    std::size_t aligned_payload_size =
        detail::align(payload_size, detail::Recordio_header::alignment);

    std::size_t record_size = header->size() + aligned_payload_size;

    if (record_size > chunk.size()) {
        if (ignore_leftover) {
            set_record_size_hint(record_size);

            return {};
        }

        throw Corrupt_header_error{fmt::format(
            "The record payload has a size of {0:n} byte(s) while the size specified in the RecordIO header is {1:n} byte(s).",
            chunk.size() - header->size(),
            aligned_payload_size)};
    }

    auto payload = chunk.subslice(header->size(), payload_size);

    chunk = chunk.subslice(record_size);

    return Record{std::move(payload), header->record_kind()};
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
