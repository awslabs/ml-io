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

#include "mlio/data_reader_base.h"

#include <utility>

#include "mlio/logger.h"

namespace mlio {
inline namespace v1 {

data_reader_base::
data_reader_base(data_reader_params &&prm) noexcept
    : params_{std::move(prm)}
{
    if (params_.bad_batch_hnd == bad_batch_handling::warn) {
        if (!logger::is_enabled_for(log_level::warning)) {
            bad_batch_handling_ = bad_batch_handling::skip;

            return;
        }
    }

    bad_batch_handling_ = params_.bad_batch_hnd;
}

intrusive_ptr<example>
data_reader_base::
read_example()
{
    if (peeked_example_) {
        return std::exchange(peeked_example_, nullptr);
    }
    return read_example_core();
}

intrusive_ptr<example> const &
data_reader_base::
peek_example()
{
    if (peeked_example_ == nullptr) {
        peeked_example_ = read_example_core();
    }
    return peeked_example_;
}

}  // namespace v1
}  // namespace mlio
