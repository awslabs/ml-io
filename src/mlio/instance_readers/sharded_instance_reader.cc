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

#include "mlio/instance_readers/sharded_instance_reader.h"

#include <utility>

#include "mlio/data_reader.h"
#include "mlio/instance.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Sharded_instance_reader::Sharded_instance_reader(const Data_reader_params &params,
                                                 std::unique_ptr<Instance_reader> &&inner)
    : params_{&params}, inner_{std::move(inner)}
{
    if (params_->shard_index >= params_->num_shards) {
        throw std::invalid_argument{"The shard index must be less than the number of shards."};
    }
}

std::optional<Instance> Sharded_instance_reader::read_instance_core()
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
        std::optional<Instance> instance = inner_->read_instance();
        if (instance == std::nullopt) {
            return {};
        }
    }

    return inner_->read_instance();
}

void Sharded_instance_reader::reset_core() noexcept
{
    inner_->reset();

    first_read_ = true;
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
