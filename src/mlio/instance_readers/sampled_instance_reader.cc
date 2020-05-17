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

#include "mlio/instance_readers/sampled_instance_reader.h"

#include <utility>

#include "mlio/data_reader.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

sampled_instance_reader::sampled_instance_reader(const data_reader_params &prm,
                                                 std::unique_ptr<instance_reader> &&inner)
    : params_{&prm}, inner_{std::move(inner)}
{
    if (params_->sample_ratio <= 0.0F || params_->sample_ratio >= 1.0F) {
        throw std::invalid_argument{"The sample ratio must be greater than 0 and less than 1."};
    }

    buffer_.reserve(num_instances_to_read_);

    buffer_pos_ = buffer_.end();
}

std::optional<instance> sampled_instance_reader::read_instance_core()
{
    if (buffer_pos_ == buffer_.end()) {
        fill_buffer_from_inner();
    }

    if (buffer_.empty()) {
        return {};
    }

    return std::exchange(*buffer_pos_++, {});
}

void sampled_instance_reader::fill_buffer_from_inner()
{
    buffer_.clear();

    while (buffer_.size() < num_instances_to_read_) {
        std::optional<instance> ins = inner_->read_instance();
        if (ins == std::nullopt) {
            break;
        }

        buffer_.emplace_back(std::move(ins));
    }

    if (!buffer_.empty()) {
        auto num_keep = static_cast<std::ptrdiff_t>(*params_->sample_ratio *
                                                    static_cast<float>(buffer_.size()));

        buffer_.erase(buffer_.begin() + num_keep, buffer_.end());
    }

    buffer_pos_ = buffer_.begin();
}

void sampled_instance_reader::reset_core() noexcept
{
    inner_->reset();

    buffer_.clear();

    buffer_pos_ = buffer_.end();
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
