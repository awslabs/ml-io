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

#include "mlio/record_readers/detail/recordio_header.h"

#include "mlio/endian.h"
#include "mlio/record_readers/corrupt_record_error.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {
namespace detail {

stdx::optional<recordio_header>
decode_recordio_header(memory_span bits)
{
    auto ints = as_span<std::uint32_t const>(bits);
    if (ints.size() < 2) {
        return {};
    }

    // There is no formal specification about the correct byte order of
    // the RecordIO format. We assume that it is always little-endian.
    constexpr std::uint32_t magic =
        (byte_order::host == byte_order::little ? 0xced7'230a : 0x0a23'd7ce);

    if (ints[0] != magic) {
        throw corrupt_header_error{
            "The header does not start with the RecordIO magic number."};
    }

    auto data = little_to_host_order(ints[1]);

    return recordio_header{data};
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
