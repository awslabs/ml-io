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
#include <string_view>

#include "mlio/config.h"
#include "mlio/span.h"

namespace mlio {
inline namespace abi_v1 {

MLIO_API
std::string_view trim(std::string_view s) noexcept;

MLIO_API
bool is_whitespace_only(std::string_view s) noexcept;

MLIO_API
inline std::string_view as_string_view(stdx::span<const std::byte> bits) noexcept
{
    auto chars = as_span<const char>(bits);

    return std::string_view{chars.data(), chars.size()};
}

}  // namespace abi_v1
}  // namespace mlio
