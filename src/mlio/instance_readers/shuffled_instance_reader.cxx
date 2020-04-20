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

#include "mlio/instance_readers/shuffled_instance_reader.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "mlio/data_reader.h"

namespace mlio {
inline namespace v1 {
namespace detail {

shuffled_instance_reader::shuffled_instance_reader(
    data_reader_params const &prm, std::unique_ptr<instance_reader> &&inner)
    : params_{&prm}
    , inner_{std::move(inner)}
    , shuffle_window_{params_->shuffle_window}
    , dist_{0, shuffle_window_ - 1}
{
    if (shuffle_window_ == 1) {
        return;
    }
    if (shuffle_window_ == 0) {
        shuffle_window_ = std::numeric_limits<std::size_t>::max();
    }
    else {
        buffer_.reserve(shuffle_window_);
    }

    if (params_->shuffle_seed != std::nullopt) {
        seed_ = *params_->shuffle_seed;

        mt_.seed(seed_);
    }
}

std::optional<instance>
shuffled_instance_reader::read_instance_core()
{
    if (shuffle_window_ == 1) {
        return inner_->read_instance();
    }

    fill_buffer_from_inner();

    if (buffer_.empty()) {
        return {};
    }

    if (inner_has_instance_) {
        return pop_random_instance_from_buffer();
    }

    instance ins = std::move(buffer_.back());

    buffer_.pop_back();

    return std::move(ins);
}

void
shuffled_instance_reader::fill_buffer_from_inner()
{
    while (inner_has_instance_ && buffer_.size() < shuffle_window_) {
        std::optional<instance> ins = inner_->read_instance();
        if (ins == std::nullopt) {
            inner_has_instance_ = false;

            std::shuffle(buffer_.begin(), buffer_.end(), mt_);

            break;
        }

        buffer_.emplace_back(std::move(*ins));
    }
}

std::optional<instance>
shuffled_instance_reader::pop_random_instance_from_buffer()
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
shuffled_instance_reader::reset_core() noexcept
{
    inner_->reset();

    buffer_.clear();

    inner_has_instance_ = true;

    // Make sure that we reset the random number generator engine to
    // its initial state if reshuffling is not requested.
    if (!params_->reshuffle_each_epoch) {
        mt_.seed(seed_);
    }
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
