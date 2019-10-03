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

#include "mlio/data_reader.h"
#include "mlio/fwd.h"
#include "mlio/instance_reader.h"
#include "mlio/optional.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class instance_batch_reader {
public:
    explicit
    instance_batch_reader(data_reader_params const &prm, instance_reader &rdr) noexcept
        : params_{&prm}, reader_{&rdr}
    {}

public:
    stdx::optional<instance_batch>
    read_instance_batch();

    void
    reset() noexcept;

private:
    data_reader_params const *params_;
    instance_reader *reader_;
    std::size_t batch_idx_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
