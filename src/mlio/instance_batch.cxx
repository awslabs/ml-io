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

#include "mlio/instance_batch.h"

namespace mlio {
inline namespace v1 {

std::size_t instance_batch::size_bytes() const
{
    if (size_bytes_ == 0) {
        for (instance const &ins : instances_) {
            size_bytes_ += ins.bits().size();
        }
    }

    return size_bytes_;
}

}  // namespace v1
}  // namespace mlio
