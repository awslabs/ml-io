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
#include <optional>

#include "mlio/data_reader.h"
#include "mlio/fwd.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Instance_batch_reader {
public:
    explicit Instance_batch_reader(const Data_reader_params &params, Instance_reader &reader);

    std::optional<Instance_batch> read_instance_batch();

    void reset() noexcept;

private:
    const Data_reader_params *params_;
    Instance_reader *reader_;
    std::size_t batch_idx_{};
};

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
