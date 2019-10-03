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

#include "mlio/record_readers/record_reader_base.h"

#include <utility>

namespace mlio {
inline namespace v1 {

stdx::optional<record>
record_reader_base::
read_record()
{
    if (peeked_record_) {
        return std::exchange(peeked_record_, stdx::nullopt);
    }
    return read_record_core();
}

stdx::optional<record> const &
record_reader_base::
peek_record()
{
    if (peeked_record_ == stdx::nullopt) {
        peeked_record_ = read_record_core();
    }
    return peeked_record_;
}

}  // namespace v1
}  // namespace mlio
