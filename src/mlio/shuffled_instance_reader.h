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

#pragma once

#include <cstddef>
#include <memory>
#include <random>
#include <vector>

#include "mlio/fwd.h"
#include "mlio/instance.h"
#include "mlio/instance_reader.h"
#include "mlio/instance_reader_base.h"
#include "mlio/optional.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class shuffled_instance_reader final : public instance_reader_base {
public:
    explicit
    shuffled_instance_reader(data_reader_params const &prm,
                             std::unique_ptr<instance_reader> &&inner);

private:
    stdx::optional<instance>
    read_instance_core() final;

    void
    buffer_instances();

    stdx::optional<instance>
    pop_random_instance_from_buffer();

public:
    void
    reset() noexcept final;

private:
    data_reader_params const *params_;
    std::unique_ptr<instance_reader> inner_;
    std::vector<instance> buffer_{};
    bool inner_has_instances_ = true;
    std::uniform_int_distribution<std::size_t> dist_;
    std::random_device rd_{};
    std::mt19937 mt_{rd_()};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
