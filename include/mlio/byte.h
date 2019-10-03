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

#if __cplusplus >= 201703L

#include <cstddef>

namespace mlio {
inline namespace v1 {
namespace stdx {

using std::byte;

using std::to_integer;

}  // namespace stdx
}  // namespace v1
}  // namespace mlio

#else

#include <type_traits>

#include "mlio/config.h"

#if defined __GNUC__ || defined __clang__
#define MLIO_MAY_ALIAS __attribute__((__may_alias__))
#else
#define MLIO_MAY_ALIAS
#endif

namespace mlio {
inline namespace v1 {
namespace stdx {

enum class MLIO_MAY_ALIAS byte : unsigned char
{};

template<class T>
MLIO_API
inline constexpr T
to_integer(byte b) noexcept
{
    static_assert(std::is_integral<T>::value, "T must be an integral type.");

    return static_cast<T>(b);
}

}  // namespace stdx
}  // namespace v1
}  // namespace mlio

#endif
