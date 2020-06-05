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
inline namespace abi_v1 {

struct MLIO_API Float_parse_options {
    const std::unordered_set<std::string> *nan_values{};
};

template<typename T>
MLIO_API
Parse_result try_parse_float(std::string_view s, T &result, const Float_parse_options &opts = {});

// clang-format off

extern template Parse_result
try_parse_float<float> (std::string_view s, float  &result, const Float_parse_options &opts = {});

extern template Parse_result
try_parse_float<double>(std::string_view s, double &result, const Float_parse_options &opts = {});

// clang-format on

struct MLIO_API Int_parse_options {
    int base{};
};

template<typename T>
MLIO_API
Parse_result
try_parse_int(std::string_view s, T &result, const Int_parse_options &opts = {}) noexcept;

// clang-format off

extern template Parse_result
try_parse_int<std::int8_t>  (std::string_view s, std::int8_t   &result, const Int_parse_options &opts = {}) noexcept;

extern template Parse_result
try_parse_int<std::int16_t> (std::string_view s, std::int16_t  &result, const Int_parse_options &opts = {}) noexcept;

extern template Parse_result
try_parse_int<std::int32_t> (std::string_view s, std::int32_t  &result, const Int_parse_options &opts = {}) noexcept;

extern template Parse_result
try_parse_int<std::int64_t> (std::string_view s, std::int64_t  &result, const Int_parse_options &opts = {}) noexcept;

extern template Parse_result
try_parse_int<std::uint8_t> (std::string_view s, std::uint8_t  &result, const Int_parse_options &opts = {}) noexcept;

extern template Parse_result
try_parse_int<std::uint16_t>(std::string_view s, std::uint16_t &result, const Int_parse_options &opts = {}) noexcept;

extern template Parse_result
try_parse_int<std::uint32_t>(std::string_view s, std::uint32_t &result, const Int_parse_options &opts = {}) noexcept;

extern template Parse_result
try_parse_int<std::uint64_t>(std::string_view s, std::uint64_t &result, const Int_parse_options &opts = {}) noexcept;

// clang-format on

MLIO_API
Parse_result try_parse_size_t(std::string_view s, std::size_t &result) noexcept;

}  // namespace abi_v1
}  // namespace mlio
