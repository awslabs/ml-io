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
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_set>

#include "mlio/config.h"
#include "mlio/parser.h"

namespace mlio {
inline namespace v1 {

struct MLIO_API float_parse_params {
    std::string_view s{};
    std::unordered_set<std::string> const *nan_values{};
};

template<typename T>
MLIO_API
parse_result try_parse_float(const float_parse_params &prm, T &result);

template<typename T>
MLIO_API
inline parse_result try_parse_float(std::string_view s, T &result)
{
    return try_parse_float(float_parse_params{s}, result);
}

extern template parse_result try_parse_float<float>(const float_parse_params &prm, float &result);

extern template parse_result try_parse_float<double>(const float_parse_params &prm, double &result);

struct MLIO_API int_parse_params {
    std::string_view s{};
    int base{};
};

template<typename T>
MLIO_API
parse_result try_parse_int(const int_parse_params &prm, T &result) noexcept;

// clang-format off

extern template parse_result
try_parse_int<std::int8_t>  (const int_parse_params &prm, std::int8_t   &result) noexcept;

extern template parse_result
try_parse_int<std::int16_t> (const int_parse_params &prm, std::int16_t  &result) noexcept;

extern template parse_result
try_parse_int<std::int32_t> (const int_parse_params &prm, std::int32_t  &result) noexcept;

extern template parse_result
try_parse_int<std::int64_t> (const int_parse_params &prm, std::int64_t  &result) noexcept;

extern template parse_result
try_parse_int<std::uint8_t> (const int_parse_params &prm, std::uint8_t  &result) noexcept;

extern template parse_result
try_parse_int<std::uint16_t>(const int_parse_params &prm, std::uint16_t &result) noexcept;

extern template parse_result
try_parse_int<std::uint32_t>(const int_parse_params &prm, std::uint32_t &result) noexcept;

extern template parse_result
try_parse_int<std::uint64_t>(const int_parse_params &prm, std::uint64_t &result) noexcept;

// clang-format on

template<typename T>
MLIO_API
inline parse_result try_parse_int(std::string_view s, T &result) noexcept
{
    return try_parse_int(int_parse_params{s}, result);
}

MLIO_API
parse_result try_parse_size_t(std::string_view s, std::size_t &result) noexcept;

}  // namespace v1
}  // namespace mlio
