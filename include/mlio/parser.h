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
#include <functional>
#include <string>
#include <string_view>
#include <unordered_set>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/device_array.h"

namespace mlio {
inline namespace abi_v1 {

enum class Parse_result { ok, failed, overflowed };

struct MLIO_API Parser_params {
    /// For a floating-point parse operation holds the list of strings
    /// that should be treated as NaN.
    std::unordered_set<std::string> nan_values{};
    /// For a number parse operation specifies the base of the number in
    /// its string representation.
    int base = 10;
};

/// Acts as a Parser for a specific data type.
///
/// @param s
///     The string to parse.
/// @param arr
///     The destination array into which to write the output of the
///     parse operation. The data type of the array must match the
///     data type of the Parser.
/// @param index
///     The index in the destination array at which to write the
///     output.
using Parser =
    std::function<Parse_result(std::string_view s, Device_array_span arr, std::size_t index)>;

/// Constructs a Parser function object.
///
/// @param dt
///     The data type for which to construct a Parser.
MLIO_API
Parser make_parser(Data_type dt, const Parser_params &params);

}  // namespace abi_v1
}  // namespace mlio
