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
#include <iostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

#include "mlio/config.h"
#include "mlio/string_view.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup tensors Tensors
/// @{

/// Specifies the data type of a @ref tensor instance.
enum class data_type {
    size,
    float16,
    float32,
    float64,
    sint8,
    sint16,
    sint32,
    sint64,
    uint8,
    uint16,
    uint32,
    uint64,
    string
};

template<data_type dt>
struct data_type_traits;

template<>
struct data_type_traits<data_type::size>    {
    using type = std::size_t;
};

template<>
struct data_type_traits<data_type::float16> {
    using type = std::uint16_t;
};

template<>
struct data_type_traits<data_type::float32> {
    using type = float;
};

template<>
struct data_type_traits<data_type::float64> {
    using type = double;
};

template<>
struct data_type_traits<data_type::sint8>   {
    using type = std::int8_t;
};

template<>
struct data_type_traits<data_type::sint16>  {
    using type = std::int16_t;
};

template<>
struct data_type_traits<data_type::sint32>  {
    using type = std::int32_t;
};

template<>
struct data_type_traits<data_type::sint64>  {
    using type = std::int64_t;
};

template<>
struct data_type_traits<data_type::uint8>   {
    using type = std::uint8_t;
};

template<>
struct data_type_traits<data_type::uint16>  {
    using type = std::uint16_t;
};

template<>
struct data_type_traits<data_type::uint32>  {
    using type = std::uint32_t;
};

template<>
struct data_type_traits<data_type::uint64>  {
    using type = std::uint64_t;
};

template<>
struct data_type_traits<data_type::string>  {
    using type = std::string;
};

template<data_type dt>
using data_type_t = typename data_type_traits<dt>::type;

template<template<data_type dt> class Op, typename... Args>
decltype(auto)
dispatch(data_type dt, Args &&...args)
{
    switch (dt) {
    case data_type::size:
        return Op<data_type::size>   ()(std::forward<Args>(args)...);
    case data_type::float16:
        return Op<data_type::float16>()(std::forward<Args>(args)...);
    case data_type::float32:
        return Op<data_type::float32>()(std::forward<Args>(args)...);
    case data_type::float64:
        return Op<data_type::float64>()(std::forward<Args>(args)...);
    case data_type::sint8:
        return Op<data_type::sint8>  ()(std::forward<Args>(args)...);
    case data_type::sint16:
        return Op<data_type::sint16> ()(std::forward<Args>(args)...);
    case data_type::sint32:
        return Op<data_type::sint32> ()(std::forward<Args>(args)...);
    case data_type::sint64:
        return Op<data_type::sint64> ()(std::forward<Args>(args)...);
    case data_type::uint8:
        return Op<data_type::uint8>  ()(std::forward<Args>(args)...);
    case data_type::uint16:
        return Op<data_type::uint16> ()(std::forward<Args>(args)...);
    case data_type::uint32:
        return Op<data_type::uint32> ()(std::forward<Args>(args)...);
    case data_type::uint64:
        return Op<data_type::uint64> ()(std::forward<Args>(args)...);
    case data_type::string:
        return Op<data_type::string> ()(std::forward<Args>(args)...);
    }

    throw std::invalid_argument{"The specified data type is not valid."};
}

/// Tries to infer the type of the data represented by the specified
/// string.
///
/// @return The inferred data type; otherwise @ref data_type::string.
MLIO_API
data_type
infer_data_type(stdx::string_view s) noexcept;

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, data_type dt)
{
    switch (dt) {
    case data_type::size:
        strm << "size";
        break;
    case data_type::float16:
        strm << "float16";
        break;
    case data_type::float32:
        strm << "float32";
        break;
    case data_type::float64:
        strm << "float64";
        break;
    case data_type::sint8:
        strm << "sint8";
        break;
    case data_type::sint16:
        strm << "sint16";
        break;
    case data_type::sint32:
        strm << "sint32";
        break;
    case data_type::sint64:
        strm << "sint64";
        break;
    case data_type::uint8:
        strm << "uint8";
        break;
    case data_type::uint16:
        strm << "uint16";
        break;
    case data_type::uint32:
        strm << "uint32";
        break;
    case data_type::uint64:
        strm << "uint64";
        break;
    case data_type::string:
        strm << "string";
        break;
    }
    return strm;
}

/// @}

}  // namespace v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::data_type> {
    inline size_t
    operator()(mlio::data_type const &dt) const noexcept
    {
        using T = underlying_type_t<mlio::data_type>;

        return hash<T>{}(static_cast<T>(dt));
    }
};

}  // namespace std
