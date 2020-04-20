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

#include <type_traits>

namespace mlio {
inline namespace v1 {
namespace detail {

template<typename T, typename = void>
struct is_container_helper : std::false_type {};

template<typename T>
struct is_container_helper<T,
                           std::void_t<decltype(std::declval<T>().data()),
                                       decltype(std::declval<T>().size())>>
    : std::true_type {};

/// Checks if T is a container having data() and size() accessors.
template<typename T>
struct is_container : detail::is_container_helper<T> {};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
