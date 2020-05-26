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

#include "mlio/data_type.h"

#include "mlio/parser.h"
#include "mlio/util/number.h"

namespace mlio {
inline namespace abi_v1 {

Data_type infer_data_type(std::string_view s) noexcept
{
    if (s.empty()) {
        return Data_type::string;
    }

    std::int64_t int64_val{};
    Parse_result r = try_parse_int<std::int64_t>(s, int64_val);
    if (r == Parse_result::ok) {
        return Data_type::int64;
    }

    if (r == Parse_result::overflowed) {
        std::uint64_t uint64_val{};
        r = try_parse_int<std::uint64_t>(s, uint64_val);
        if (r == Parse_result::ok) {
            return Data_type::uint64;
        }
    }

    double double_val{};
    r = try_parse_float<double>(s, double_val);
    if (r == Parse_result::ok) {
        return Data_type::float64;
    }

    return Data_type::string;
}

}  // namespace abi_v1
}  // namespace mlio
