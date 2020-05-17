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

#include "mlio/fwd.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

std::optional<record> read_line(memory_slice &chunk,
                                bool ignore_leftover,
                                std::optional<std::size_t> max_line_length = {});

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
