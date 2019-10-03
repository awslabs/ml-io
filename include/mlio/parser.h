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
#include <functional>
#include <string>
#include <unordered_set>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/device_array.h"
#include "mlio/string_view.h"

namespace mlio {
inline namespace v1 {

enum class parse_result {
    ok,
    failed,
    overflowed
};

struct MLIO_API parser_params {
    /// For a floating-point parse operation holds the list of strings
    /// that should be treated as NaN.
    std::unordered_set<std::string> nan_values{};
    /// For a number parse operation specifies the base of the number in
    /// its string representation.
    int base = 10;
};

/// Acts as a parser for a specific data type.
///
/// @param s
///     The string to parse.
/// @param arr
///     The destination array into which to write the output of the
///     parse operation. The data type of the array must match the
///     data type of the parser.
/// @param index
///     The index in the destination array at which to write the
///     output.
using parser = std::function<
    parse_result(stdx::string_view s, device_array_span arr, std::size_t index)>;

/// Constructs a parser function object.
///
/// @param dt
///     The data type for which to construct a parser.
MLIO_API
parser
make_parser(data_type dt, parser_params const &prm);

}  // namespace v1
}  // namespace mlio
