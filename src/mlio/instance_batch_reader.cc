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

#include "mlio/instance_batch_reader.h"

#include <utility>
#include <vector>

#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/instance_readers/instance_reader.h"
#include "mlio/logger.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Instance_batch_reader::Instance_batch_reader(const Data_reader_params &params,
                                             Instance_reader &reader)
    : params_{&params}, reader_{&reader}
{
    if (params_->batch_size == 0) {
        throw std::invalid_argument{"The batch size must be greater than zero."};
    }
}

std::optional<Instance_batch> Instance_batch_reader::read_instance_batch()
{
    std::vector<Instance> instances{};
    instances.reserve(params_->batch_size);

    for (std::size_t i = 0; i < params_->batch_size; i++) {
        std::optional<Instance> instance = reader_->read_instance();
        if (instance == std::nullopt) {
            break;
        }

        instances.emplace_back(std::move(*instance));
    }

    if (instances.empty()) {
        return {};
    }

    if (instances.size() != params_->batch_size) {
        if (params_->last_example_handling == Last_example_handling::drop) {
            return {};
        }
        if (params_->last_example_handling == Last_example_handling::drop_warn) {
            logger::warn(
                "The last example has been dropped as it had only {0:n} instance(s) while the batch size is {1:n}.",
                instances.size(),
                params_->batch_size);

            return {};
        }
        if (params_->last_example_handling == Last_example_handling::pad_warn) {
            logger::warn(
                "The last example has been padded as it had only {0:n} instance(s) while the batch size is {1:n}.",
                instances.size(),
                params_->batch_size);
        }
    }

    std::size_t size{};
    if (params_->last_example_handling == Last_example_handling::pad) {
        size = params_->batch_size;
    }
    else {
        size = instances.size();
    }

    return Instance_batch{batch_idx_++, std::move(instances), size};
}

void Instance_batch_reader::reset() noexcept
{
    reader_->reset();

    batch_idx_ = 0;
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
