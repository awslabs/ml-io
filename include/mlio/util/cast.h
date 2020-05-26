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
#include <type_traits>
#include <utility>

#include "mlio/config.h"
#include "mlio/type_traits.h"

namespace mlio {
inline namespace abi_v1 {
namespace stdx {

template<typename Container>
inline constexpr std::ptrdiff_t ssize(const Container &container) noexcept
{
    return static_cast<std::ptrdiff_t>(container.size());
}

}  // namespace stdx

namespace detail {

template<typename T, typename U>
struct Is_same_signedness
    : public std::bool_constant<std::is_signed<T>::value == std::is_signed<U>::value> {};

}  // namespace detail

MLIO_API
inline constexpr std::size_t as_size(std::ptrdiff_t d) noexcept
{
    return static_cast<std::size_t>(d);
}

MLIO_API
inline constexpr std::ptrdiff_t as_ssize(std::size_t s) noexcept
{
    return static_cast<std::ptrdiff_t>(s);
}

template<typename T, typename U>
MLIO_API
inline constexpr T narrow_cast(U &&u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}

template<typename T, typename U>
MLIO_API
inline constexpr bool try_narrow(U u, T &t) noexcept
{
    t = narrow_cast<T>(u);

    if (static_cast<U>(t) != u) {
        return false;
    }
    if (!detail::Is_same_signedness<T, U>::value && ((t < T{}) != (u < U{}))) {
        return false;
    }
    return true;
}

}  // namespace abi_v1
}  // namespace mlio
