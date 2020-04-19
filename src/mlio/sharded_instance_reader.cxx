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

#include "mlio/sharded_instance_reader.h"

#include <utility>

#include "mlio/data_reader.h"

namespace mlio {
inline namespace v1 {
namespace detail {

sharded_instance_reader::sharded_instance_reader(
    data_reader_params const &prm, std::unique_ptr<instance_reader> &&inner)
    : params_{&prm}, inner_{std::move(inner)}
{
    if (params_->shard_index >= params_->num_shards) {
        throw std::invalid_argument{
            "The shard index must be less than the number of shards."};
    }
}

std::optional<instance>
sharded_instance_reader::read_instance_core()
{
    std::size_t num_instances_to_skip{};

    if (first_read_) {
        first_read_ = false;

        num_instances_to_skip = params_->shard_index;
    }
    else {
        num_instances_to_skip = params_->num_shards - 1;
    }

    for (std::size_t i = 0; i < num_instances_to_skip; i++) {
        std::optional<instance> ins = inner_->read_instance();
        if (ins == std::nullopt) {
            return {};
        }
    }

    return inner_->read_instance();
}

void
sharded_instance_reader::reset() noexcept
{
    inner_->reset();

    first_read_ = true;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
