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

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

#include "mlio/config.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup tensors Tensors
/// @{

/// Specifies the data type of a @ref Tensor Instance.
enum class Data_type {
    size,
    float16,
    float32,
    float64,
    int8,
    int16,
    int32,
    int64,
    uint8,
    uint16,
    uint32,
    uint64,
    string
};

// clang-format off

template<Data_type dt>
struct Data_type_traits;

template<>
struct Data_type_traits<Data_type::size>    {
    using type = std::size_t;
};

template<>
struct Data_type_traits<Data_type::float16> {
    using type = std::uint16_t;
};

template<>
struct Data_type_traits<Data_type::float32> {
    using type = float;
};

template<>
struct Data_type_traits<Data_type::float64> {
    using type = double;
};

template<>
struct Data_type_traits<Data_type::int8>    {
    using type = std::int8_t;
};

template<>
struct Data_type_traits<Data_type::int16>   {
    using type = std::int16_t;
};

template<>
struct Data_type_traits<Data_type::int32>   {
    using type = std::int32_t;
};

template<>
struct Data_type_traits<Data_type::int64>   {
    using type = std::int64_t;
};

template<>
struct Data_type_traits<Data_type::uint8>   {
    using type = std::uint8_t;
};

template<>
struct Data_type_traits<Data_type::uint16>  {
    using type = std::uint16_t;
};

template<>
struct Data_type_traits<Data_type::uint32>  {
    using type = std::uint32_t;
};

template<>
struct Data_type_traits<Data_type::uint64>  {
    using type = std::uint64_t;
};

template<>
struct Data_type_traits<Data_type::string>  {
    using type = std::string;
};

template<Data_type dt>
using data_type_t = typename Data_type_traits<dt>::type;

template<template<Data_type dt> class Op, typename... Args>
decltype(auto)
dispatch(Data_type dt, Args &&...args)
{
    switch (dt) {
    case Data_type::size:
        return Op<Data_type::size>   ()(std::forward<Args>(args)...);
    case Data_type::float16:
        return Op<Data_type::float16>()(std::forward<Args>(args)...);
    case Data_type::float32:
        return Op<Data_type::float32>()(std::forward<Args>(args)...);
    case Data_type::float64:
        return Op<Data_type::float64>()(std::forward<Args>(args)...);
    case Data_type::int8:
        return Op<Data_type::int8>   ()(std::forward<Args>(args)...);
    case Data_type::int16:
        return Op<Data_type::int16>  ()(std::forward<Args>(args)...);
    case Data_type::int32:
        return Op<Data_type::int32>  ()(std::forward<Args>(args)...);
    case Data_type::int64:
        return Op<Data_type::int64>  ()(std::forward<Args>(args)...);
    case Data_type::uint8:
        return Op<Data_type::uint8>  ()(std::forward<Args>(args)...);
    case Data_type::uint16:
        return Op<Data_type::uint16> ()(std::forward<Args>(args)...);
    case Data_type::uint32:
        return Op<Data_type::uint32> ()(std::forward<Args>(args)...);
    case Data_type::uint64:
        return Op<Data_type::uint64> ()(std::forward<Args>(args)...);
    case Data_type::string:
        return Op<Data_type::string> ()(std::forward<Args>(args)...);
    }

    throw std::invalid_argument{"The specified data type is not valid."};
}

// clang-format on

/// Tries to infer the type of the data represented by the specified
/// string.
///
/// @return The inferred data type; otherwise @ref Data_type::string.
MLIO_API
Data_type infer_data_type(std::string_view s) noexcept;

MLIO_API
inline std::ostream &operator<<(std::ostream &s, Data_type dt)
{
    switch (dt) {
    case Data_type::size:
        s << "size";
        break;
    case Data_type::float16:
        s << "float16";
        break;
    case Data_type::float32:
        s << "float32";
        break;
    case Data_type::float64:
        s << "float64";
        break;
    case Data_type::int8:
        s << "int8";
        break;
    case Data_type::int16:
        s << "int16";
        break;
    case Data_type::int32:
        s << "int32";
        break;
    case Data_type::int64:
        s << "int64";
        break;
    case Data_type::uint8:
        s << "uint8";
        break;
    case Data_type::uint16:
        s << "uint16";
        break;
    case Data_type::uint32:
        s << "uint32";
        break;
    case Data_type::uint64:
        s << "uint64";
        break;
    case Data_type::string:
        s << "string";
        break;
    }
    return s;
}

/// @}

}  // namespace abi_v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::Data_type> {
    inline size_t operator()(const mlio::Data_type &dt) const noexcept
    {
        using T = underlying_type_t<mlio::Data_type>;

        return hash<T>{}(static_cast<T>(dt));
    }
};

}  // namespace std
