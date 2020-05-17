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

#include <functional>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {
namespace detail {

template<typename T>
MLIO_API
inline void hash_combine(std::size_t &seed, const T &v)
{
    // Taken from the Boost hash_combine function.
    seed ^= std::hash<T>{}(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

template<typename Cont>
MLIO_API
inline void hash_range(std::size_t &seed, const Cont &cont)
{
    for (const auto &e : cont) {
        hash_combine(seed, e);
    }
}

template<typename Cont>
MLIO_API
inline std::size_t hash_range(const Cont &cont)
{
    std::size_t seed = 0;

    hash_range(seed, cont);

    return seed;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
