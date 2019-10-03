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

#include "mlio/record_readers/text_line_record_reader.h"

#include "mlio/memory/memory_slice.h"
#include "mlio/optional.h"
#include "mlio/record_readers/detail/text_line.h"
#include "mlio/record_readers/record.h"

using mlio::detail::read_line;

namespace mlio {
inline namespace v1 {
namespace detail {

stdx::optional<record>
text_line_record_reader::
decode_text_record(memory_slice &chunk, bool ignore_leftover)
{
    if (chunk.empty()) {
        return {};
    }

    if (skip_blank_) {
        while (!chunk.empty()) {
            stdx::optional<record> rec = read_line(chunk, ignore_leftover);
            if (rec == stdx::nullopt || !rec->payload().empty()) {
                return rec;
            }
        }

        return {};
    }

    return detail::read_line(chunk, ignore_leftover);
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
