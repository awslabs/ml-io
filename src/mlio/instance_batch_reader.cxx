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

#include "mlio/instance_batch_reader.h"

#include <utility>
#include <vector>

#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/logger.h"

namespace mlio {
inline namespace v1 {
namespace detail {

instance_batch_reader::instance_batch_reader(data_reader_params const &prm,
                                             instance_reader &rdr)
    : params_{&prm}, reader_{&rdr}
{
    if (params_->batch_size == 0) {
        throw std::invalid_argument{
            "The batch size must be greater than zero."};
    }

    if (params_->subsample_ratio == std::nullopt) {
        return;
    }

    float sbr = *params_->subsample_ratio;
    if (sbr <= 0.0F || sbr >= 1.0F) {
        throw std::invalid_argument{"The subsampling ratio must be greater "
                                    "than zero and less than one."};
    }

    auto tmp = static_cast<float>(params_->batch_size);

    num_instances_to_skip_ =
        static_cast<std::size_t>(tmp / sbr) - params_->batch_size;
}

std::optional<instance_batch>
instance_batch_reader::read_instance_batch()
{
    std::vector<instance> instances{};
    instances.reserve(params_->batch_size);

    for (std::size_t i = 0; i < params_->batch_size; i++) {
        std::optional<instance> ins = reader_->read_instance();
        if (ins == std::nullopt) {
            break;
        }

        instances.emplace_back(std::move(*ins));
    }

    if (instances.size() != params_->batch_size) {
        if (params_->last_batch_hnd == last_batch_handling::drop) {
            logger::debug("The last batch has been dropped.");

            return {};
        }
    }

    if (instances.empty()) {
        return {};
    }

    for (std::size_t i = 0; i < num_instances_to_skip_; i++) {
        if (reader_->read_instance() == std::nullopt) {
            break;
        }
    }

    std::size_t size;
    if (params_->last_batch_hnd == last_batch_handling::pad) {
        size = params_->batch_size;
    }
    else {
        size = instances.size();
    }

    return instance_batch{batch_idx_++, std::move(instances), size};
}

void
instance_batch_reader::reset() noexcept
{
    reader_->reset();

    batch_idx_ = 0;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
