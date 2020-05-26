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

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <random>
#include <vector>

#include "mlio/fwd.h"
#include "mlio/instance.h"
#include "mlio/instance_readers/instance_reader.h"
#include "mlio/instance_readers/instance_reader_base.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Shuffled_instance_reader final : public Instance_reader_base {
public:
    explicit Shuffled_instance_reader(const Data_reader_params &params,
                                      std::unique_ptr<Instance_reader> &&inner);

private:
    std::optional<Instance> read_instance_core() final;

    void fill_buffer_from_inner();

    std::optional<Instance> pop_random_instance_from_buffer();

    void reset_core() noexcept final;

    const Data_reader_params *params_;
    std::unique_ptr<Instance_reader> inner_;
    std::size_t shuffle_window_;
    std::vector<Instance> buffer_{};
    bool inner_has_instance_ = true;
    std::random_device rd_{};
    std::uint_fast64_t seed_{rd_()};
    std::mt19937_64 mt_{seed_};
    std::uniform_int_distribution<std::size_t> dist_;
};

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
