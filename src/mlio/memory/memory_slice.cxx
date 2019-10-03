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

#include "mlio/memory/memory_slice.h"

#include <stdexcept>

namespace mlio {
inline namespace v1 {

void
memory_slice::
validate_range(memory_block::iterator first, memory_block::iterator last) const
{
    if (first > last) {
        throw std::invalid_argument{"The specified range is invalid."};
    }

    if (first < beg_ || last > end_) {
        throw std::invalid_argument{
            "The specified range does not fall within the slice."};
    }
}

}  // namespace v1
}  // namespace mlio
