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

#include "mlio/util/number.h"

#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>

#include <absl/strings/numbers.h>
#include <absl/strings/string_view.h>

#include "mlio/util/string.h"

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

absl::string_view
to_absl_sv(stdx::string_view const &s) noexcept
{
    return absl::string_view{s.data(), s.size()};
}

template<typename T, typename = void>
struct parser_traits;

template<>
struct parser_traits<float> {
    static constexpr bool(*parse_func)(absl::string_view s, float  *result)
        = absl::SimpleAtof;
};

template<>
struct parser_traits<double> {
    static constexpr bool(*parse_func)(absl::string_view s, double *result)
        = absl::SimpleAtod;
};

template<typename T>
struct parser_traits<T, std::enable_if_t<std::is_integral<T>::value>> {
    static constexpr bool(*parse_func)(absl::string_view s, T      *result)
        = absl::SimpleAtoi;
};

template<typename T>
parse_result
try_parse_float_core(float_parse_params const &prm, T &result)
{
    T v = 0.0;

    if (!parser_traits<T>::parse_func(to_absl_sv(prm.s), &v)) {
        auto *nan_values = prm.nan_values;
        if (nan_values != nullptr && !nan_values->empty()) {
            auto trimmed = static_cast<std::string>(trim(prm.s));
            if (nan_values->find(trimmed) != nan_values->end()) {
                result = std::numeric_limits<T>::quiet_NaN();

                return parse_result::ok;
            }
        }
        return parse_result::failed;
    }

    if (std::isinf(v)) {
        return parse_result::overflowed;
    }

    result = v;

    return parse_result::ok;
}

template<typename T>
std::enable_if_t<sizeof(T) >= 4, parse_result>
try_parse_int_core(int_parse_params const &prm, T &result) noexcept
{
    // Do not use 0; otherwise we cannot distinguish between a failure
    // and an overflow for unsigned types.
    T v = 1;

    if (!parser_traits<T>::parse_func(to_absl_sv(prm.s), &v)) {
        if (v == std::numeric_limits<T>::max() ||
            v == std::numeric_limits<T>::min()) {

            return parse_result::overflowed;
        }
        return parse_result::failed;
    }

    result = v;

    return parse_result::ok;
}

template<typename T>
std::enable_if_t<sizeof(T) <= 2, parse_result>
try_parse_int_core(int_parse_params const &prm, T &result) noexcept
{
    using U = std::conditional_t<
        std::is_signed<T>::value, std::int32_t, std::uint32_t>;

    U v;

    parse_result r = try_parse_int_core<U>(prm, v);
    if (r != parse_result::ok) {
        return r;
    }

    if (v < std::numeric_limits<T>::min() ||
        v > std::numeric_limits<T>::max()) {

        return parse_result::overflowed;
    }

    result = static_cast<T>(v);

    return parse_result::ok;
}

}  // namespace
}  // namespace detail

template<typename T>
inline parse_result
try_parse_float(float_parse_params const &prm, T &result)
{
    return detail::try_parse_float_core(prm, result);
}

template
parse_result
try_parse_float<float> (float_parse_params const &prm, float  &result);

template
parse_result
try_parse_float<double>(float_parse_params const &prm, double &result);

template<typename T>
inline parse_result
try_parse_int(int_parse_params const &prm, T &result) noexcept
{
    return detail::try_parse_int_core(prm, result);
}

template
parse_result
try_parse_int<std::int8_t>  (int_parse_params const &prm, std::int8_t   &result) noexcept;

template
parse_result
try_parse_int<std::int16_t> (int_parse_params const &prm, std::int16_t  &result) noexcept;

template
parse_result
try_parse_int<std::int32_t> (int_parse_params const &prm, std::int32_t  &result) noexcept;

template
parse_result
try_parse_int<std::int64_t> (int_parse_params const &prm, std::int64_t  &result) noexcept;

template
parse_result
try_parse_int<std::uint8_t> (int_parse_params const &prm, std::uint8_t  &result) noexcept;

template
parse_result
try_parse_int<std::uint16_t>(int_parse_params const &prm, std::uint16_t &result) noexcept;

template
parse_result
try_parse_int<std::uint32_t>(int_parse_params const &prm, std::uint32_t &result) noexcept;

template
parse_result
try_parse_int<std::uint64_t>(int_parse_params const &prm, std::uint64_t &result) noexcept;

parse_result
try_parse_size_t(stdx::string_view s, std::size_t &result) noexcept
{
    return try_parse_int(s, result);
}

}  // namespace v1
}  // namespace mlio
