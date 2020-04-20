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

#include "mlio/instance_readers/instance_reader_base.h"

namespace mlio {
inline namespace v1 {
namespace detail {

std::optional<instance>
instance_reader_base::read_instance()
{
    std::optional<instance> ins{};
    if (peeked_instance_) {
        ins = std::exchange(peeked_instance_, std::nullopt);
    }
    else {
        ins = read_instance_core();
    }

    if (ins) {
        num_bytes_read_ += ins->bits().size();
    }

    return ins;
}

std::optional<instance>
instance_reader_base::peek_instance()
{
    if (peeked_instance_ == std::nullopt) {
        peeked_instance_ = read_instance_core();
    }
    return peeked_instance_;
}

void
instance_reader_base::reset() noexcept
{
    reset_core();

    peeked_instance_ = {};

    num_bytes_read_ = 0;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
