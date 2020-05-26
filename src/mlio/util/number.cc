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

#include "mlio/util/number.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <absl/strings/numbers.h>

#include "mlio/util/string.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

template<typename T, typename = void>
struct parser_traits;

template<>
struct parser_traits<float> {
    static constexpr bool (*parse_func)(absl::string_view s, float *result) = absl::SimpleAtof;
};

template<>
struct parser_traits<double> {
    static constexpr bool (*parse_func)(absl::string_view s, double *result) = absl::SimpleAtod;
};

template<typename T>
struct parser_traits<T, std::enable_if_t<std::is_integral_v<T>>> {
    static constexpr bool (*parse_func)(absl::string_view s, T *result) = absl::SimpleAtoi;
};

template<typename T>
Parse_result try_parse_float_core(const Float_parse_params &params, T &result)
{
    T v = 0.0;

    if (!parser_traits<T>::parse_func(params.s, &v)) {
        auto *nan_values = params.nan_values;
        if (nan_values != nullptr && !nan_values->empty()) {
            auto trimmed = static_cast<std::string>(trim(params.s));
            if (nan_values->find(trimmed) != nan_values->end()) {
                result = std::numeric_limits<T>::quiet_NaN();

                return Parse_result::ok;
            }
        }
        return Parse_result::failed;
    }

    if (std::isinf(v)) {
        return Parse_result::overflowed;
    }

    result = v;

    return Parse_result::ok;
}

template<typename T>
std::enable_if_t<sizeof(T) >= 4, Parse_result>
try_parse_int_core(const Int_parse_params &params, T &result) noexcept
{
    // Do not use 0; otherwise we cannot distinguish between a failure
    // and an overflow for unsigned types.
    T v = 1;

    if (!parser_traits<T>::parse_func(params.s, &v)) {
        if (v == std::numeric_limits<T>::max() || v == std::numeric_limits<T>::min()) {
            return Parse_result::overflowed;
        }
        return Parse_result::failed;
    }

    result = v;

    return Parse_result::ok;
}

template<typename T>
std::enable_if_t<sizeof(T) <= 2, Parse_result>
try_parse_int_core(const Int_parse_params &params, T &result) noexcept
{
    using U = std::conditional_t<std::is_signed_v<T>, std::int32_t, std::uint32_t>;

    U v;

    Parse_result r = try_parse_int_core<U>(params, v);
    if (r != Parse_result::ok) {
        return r;
    }

    if (v < std::numeric_limits<T>::min() || v > std::numeric_limits<T>::max()) {
        return Parse_result::overflowed;
    }

    result = static_cast<T>(v);

    return Parse_result::ok;
}

}  // namespace
}  // namespace detail

template<typename T>
inline Parse_result try_parse_float(const Float_parse_params &params, T &result)
{
    return detail::try_parse_float_core(params, result);
}

template Parse_result try_parse_float<float>(const Float_parse_params &params, float &result);

template Parse_result try_parse_float<double>(const Float_parse_params &params, double &result);

template<typename T>
inline Parse_result try_parse_int(const Int_parse_params &params, T &result) noexcept
{
    return detail::try_parse_int_core(params, result);
}

// clang-format off

template Parse_result
try_parse_int<std::int8_t>  (const Int_parse_params &params, std::int8_t   &result) noexcept;

template Parse_result
try_parse_int<std::int16_t> (const Int_parse_params &params, std::int16_t  &result) noexcept;

template Parse_result
try_parse_int<std::int32_t> (const Int_parse_params &params, std::int32_t  &result) noexcept;

template Parse_result
try_parse_int<std::int64_t> (const Int_parse_params &params, std::int64_t  &result) noexcept;

template Parse_result
try_parse_int<std::uint8_t> (const Int_parse_params &params, std::uint8_t  &result) noexcept;

template Parse_result
try_parse_int<std::uint16_t>(const Int_parse_params &params, std::uint16_t &result) noexcept;

template Parse_result
try_parse_int<std::uint32_t>(const Int_parse_params &params, std::uint32_t &result) noexcept;

template Parse_result
try_parse_int<std::uint64_t>(const Int_parse_params &params, std::uint64_t &result) noexcept;

// clang-format on

Parse_result try_parse_size_t(std::string_view s, std::size_t &result) noexcept
{
    return try_parse_int(s, result);
}

}  // namespace abi_v1
}  // namespace mlio
