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
#include <optional>
#include <vector>

#include "mlio/fwd.h"
#include "mlio/instance.h"
#include "mlio/instance_readers/instance_reader.h"
#include "mlio/instance_readers/instance_reader_base.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class sampled_instance_reader final : public instance_reader_base {
    static constexpr std::size_t num_instances_to_read_ = 100;

public:
    explicit sampled_instance_reader(data_reader_params const &prm,
                                     std::unique_ptr<instance_reader> &&inner);

private:
    std::optional<instance>
    read_instance_core() final;

    void
    fill_buffer_from_inner();

    void
    reset_core() noexcept final;

private:
    data_reader_params const *params_;
    std::unique_ptr<instance_reader> inner_;
    std::vector<std::optional<instance>> buffer_{};
    std::vector<std::optional<instance>>::iterator buffer_pos_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
