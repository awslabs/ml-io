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

#include "mlio/ranged_instance_reader.h"

#include <utility>

#include "mlio/data_reader.h"

namespace mlio {
inline namespace v1 {
namespace detail {

ranged_instance_reader::ranged_instance_reader(
    data_reader_params const &prm, std::unique_ptr<instance_reader> &&inner)
    : params_{&prm}, inner_{std::move(inner)}
{}

std::optional<instance>
ranged_instance_reader::read_instance_core()
{
    if (first_read_) {
        first_read_ = false;

        for (std::size_t i = 0; i < params_->num_instances_to_skip; i++) {
            std::optional<instance> ins = inner_->read_instance();
            if (ins == std::nullopt) {
                return {};
            }
        }
    }

    if (should_stop_reading()) {
        return {};
    }

    std::optional<instance> ins = inner_->read_instance();
    if (ins == std::nullopt) {
        return {};
    }

    num_instances_read_++;

    return ins;
}

inline bool
ranged_instance_reader::should_stop_reading() const noexcept
{
    if (params_->num_instances_to_read == std::nullopt) {
        return false;
    }

    return num_instances_read_ == *params_->num_instances_to_read;
}

void
ranged_instance_reader::reset_core() noexcept
{
    inner_->reset();

    first_read_ = true;

    num_instances_read_ = 0;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
