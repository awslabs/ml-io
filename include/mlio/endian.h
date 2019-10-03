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

#include <cstdint>

#include "mlio/config.h"

#ifdef MLIO_PLATFORM_WIN32
#   define MLIO_BYTE_ORDER_LITTLE 0
#   define MLIO_BYTE_ORDER_BIG    1
#   define MLIO_BYTE_ORDER_HOST   MLIO_BYTE_ORDER_LITTLE
#else
#   define MLIO_BYTE_ORDER_LITTLE __ORDER_LITTLE_ENDIAN__
#   define MLIO_BYTE_ORDER_BIG    __ORDER_BIG_ENDIAN__
#   define MLIO_BYTE_ORDER_HOST   __BYTE_ORDER__
#endif

namespace mlio {
inline namespace v1 {
namespace detail {

MLIO_API
inline std::uint16_t
reverse_bytes(std::uint16_t value) noexcept
{
    return __builtin_bswap16(value);
}

MLIO_API
inline std::uint32_t
reverse_bytes(std::uint32_t value) noexcept
{
    return __builtin_bswap32(value);
}

MLIO_API
inline std::uint64_t
reverse_bytes(std::uint64_t value) noexcept
{
    return __builtin_bswap64(value);
}

}  // namespace detail

#if MLIO_BYTE_ORDER_HOST == MLIO_BYTE_ORDER_LITTLE

MLIO_API
inline std::uint16_t
little_to_host_order(std::uint16_t value) noexcept
{
    return value;
}

MLIO_API
inline std::uint32_t
little_to_host_order(std::uint32_t value) noexcept
{
    return value;
}

MLIO_API
inline std::uint64_t
little_to_host_order(std::uint64_t value) noexcept
{
    return value;
}

#else

MLIO_API
inline std::uint16_t
little_to_host_order(std::uint16_t value) noexcept
{
    return detail::reverse_bytes(value);
}

MLIO_API
inline std::uint32_t
little_to_host_order(std::uint32_t value) noexcept
{
    return detail::reverse_bytes(value);
}

MLIO_API
inline std::uint64_t
little_to_host_order(std::uint64_t value) noexcept
{
    return detail::reverse_bytes(value);
}

#endif

enum class byte_order
{
    little = MLIO_BYTE_ORDER_LITTLE,
    big    = MLIO_BYTE_ORDER_BIG,
    host   = MLIO_BYTE_ORDER_HOST
};

}  // namespace v1
}  // namespace mlio
