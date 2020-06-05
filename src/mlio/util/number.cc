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

// clang-format off

template<>
struct parser_traits<float> {
    static constexpr bool (*parse_func)(absl::string_view s, float  *result) = absl::SimpleAtof;
};

template<>
struct parser_traits<double> {
    static constexpr bool (*parse_func)(absl::string_view s, double *result) = absl::SimpleAtod;
};

template<typename T>
struct parser_traits<T, std::enable_if_t<std::is_integral_v<T>>> {
    static constexpr bool (*parse_func)(absl::string_view s, T      *result) = absl::SimpleAtoi;
};

// clang-format on

template<typename T>
Parse_result try_parse_float_core(std::string_view s, T &result, const Float_parse_options &opts)
{
    T v = 0.0;

    if (!parser_traits<T>::parse_func(s, &v)) {
        auto *nan_values = opts.nan_values;
        if (nan_values != nullptr && !nan_values->empty()) {
            auto trimmed = static_cast<std::string>(trim(s));
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
try_parse_int_core(std::string_view s, T &result, const Int_parse_options &) noexcept
{
    // Do not use 0; otherwise we cannot distinguish between a failure
    // and an overflow for unsigned types.
    T v = 1;

    if (!parser_traits<T>::parse_func(s, &v)) {
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
try_parse_int_core(std::string_view s, T &result, const Int_parse_options &opts) noexcept
{
    using U = std::conditional_t<std::is_signed_v<T>, std::int32_t, std::uint32_t>;

    U v;

    Parse_result r = try_parse_int_core<U>(s, v, opts);
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
inline Parse_result try_parse_float(std::string_view s, T &result, const Float_parse_options &opts)
{
    return detail::try_parse_float_core(s, result, opts);
}

// clang-format off

template Parse_result
try_parse_float<float> (std::string_view s, float  &result, const Float_parse_options &opts);

template Parse_result
try_parse_float<double>(std::string_view s, double &result, const Float_parse_options &opts);

// clang-format on

template<typename T>
inline Parse_result
try_parse_int(std::string_view s, T &result, const Int_parse_options &opts) noexcept
{
    return detail::try_parse_int_core(s, result, opts);
}

// clang-format off

template Parse_result
try_parse_int<std::int8_t>  (std::string_view s, std::int8_t   &result, const Int_parse_options &opts) noexcept;

template Parse_result
try_parse_int<std::int16_t> (std::string_view s, std::int16_t  &result, const Int_parse_options &opts) noexcept;

template Parse_result
try_parse_int<std::int32_t> (std::string_view s, std::int32_t  &result, const Int_parse_options &opts) noexcept;

template Parse_result
try_parse_int<std::int64_t> (std::string_view s, std::int64_t  &result, const Int_parse_options &opts) noexcept;

template Parse_result
try_parse_int<std::uint8_t> (std::string_view s, std::uint8_t  &result, const Int_parse_options &opts) noexcept;

template Parse_result
try_parse_int<std::uint16_t>(std::string_view s, std::uint16_t &result, const Int_parse_options &opts) noexcept;

template Parse_result
try_parse_int<std::uint32_t>(std::string_view s, std::uint32_t &result, const Int_parse_options &opts) noexcept;

template Parse_result
try_parse_int<std::uint64_t>(std::string_view s, std::uint64_t &result, const Int_parse_options &opts) noexcept;

// clang-format on

Parse_result try_parse_size_t(std::string_view s, std::size_t &result) noexcept
{
    return try_parse_int(s, result);
}

}  // namespace abi_v1
}  // namespace mlio
