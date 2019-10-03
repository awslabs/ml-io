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

#include "mlio/shuffled_instance_reader.h"

#include <algorithm>
#include <utility>

#include "mlio/data_reader.h"
#include "mlio/not_supported_error.h"

namespace mlio {
inline namespace v1 {
namespace detail {

shuffled_instance_reader::
shuffled_instance_reader(data_reader_params const &prm,
                         std::unique_ptr<instance_reader> &&inner)
    : params_{&prm}, inner_{std::move(inner)}, dist_{0, params_->shuffle_window - 1}
{
    if (params_->shuffle_window == 0) {
        throw not_supported_error{
            "Perfect shuffling is not implemented yet! Please specify a "
            "shuffle window greater than zero."};
    }

    if (params_->shuffle_window == 1) {
        return;
    }

    buffer_.reserve(params_->shuffle_window);

    if (params_->shuffle_seed != stdx::nullopt) {
        mt_.seed(*params_->shuffle_seed);
    }
}

stdx::optional<instance>
shuffled_instance_reader::
read_instance_core()
{
    if (params_->shuffle_window == 1) {
        return inner_->read_instance();
    }

    buffer_instances();

    if (buffer_.empty()) {
        return {};
    }

    if (inner_has_instances_) {
        return pop_random_instance_from_buffer();
    }

    instance ins = std::move(buffer_.back());

    buffer_.pop_back();

    return std::move(ins);
}

void
shuffled_instance_reader::
buffer_instances()
{
    while (inner_has_instances_ && buffer_.size() < params_->shuffle_window) {
        stdx::optional<instance> ins = inner_->read_instance();
        if (ins == stdx::nullopt) {
            inner_has_instances_ = false;

            std::shuffle(buffer_.begin(), buffer_.end(), mt_);

            break;
        }

        buffer_.emplace_back(std::move(*ins));
    }
}

stdx::optional<instance>
shuffled_instance_reader::
pop_random_instance_from_buffer()
{
    std::size_t random_idx = dist_(mt_);

    instance ins = std::move(buffer_[random_idx]);

    if (random_idx != buffer_.size() - 1) {
        buffer_[random_idx] = std::move(buffer_.back());
    }

    buffer_.pop_back();

    return std::move(ins);
}

void
shuffled_instance_reader::
reset() noexcept
{
    inner_->reset();

    buffer_.clear();

    inner_has_instances_ = true;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
