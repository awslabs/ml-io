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

#include "mlio/instance_reader_base.h"

namespace mlio {
inline namespace v1 {
namespace detail {

stdx::optional<instance>
instance_reader_base::
read_instance()
{
    if (peeked_instance_) {
        return std::exchange(peeked_instance_, stdx::nullopt);
    }
    return read_instance_core();
}

stdx::optional<instance> const &
instance_reader_base::
peek_instance()
{
    if (peeked_instance_ == stdx::nullopt) {
        peeked_instance_ = read_instance_core();
    }
    return peeked_instance_;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
