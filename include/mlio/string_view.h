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

#include <ciso646>

#if __cplusplus >= 201703L || (defined(_LIBCPP_VERSION) && _LIBCPP_VERSION >= 5000)

#include <string_view>  // IWYU pragma: export

namespace mlio {
inline namespace v1 {
namespace stdx {

using std::string_view;

using std::operator==;
using std::operator!=;
using std::operator<;
using std::operator<=;
using std::operator>;
using std::operator>=;

using std::operator<<;

}  // namespace stdx
}  // namespace v1
}  // namespace mlio

#else

#include <experimental/string_view>  // IWYU pragma: export

namespace mlio {
inline namespace v1 {
namespace stdx {

using std::experimental::string_view;

using std::experimental::operator==;
using std::experimental::operator!=;
using std::experimental::operator<;
using std::experimental::operator<=;
using std::experimental::operator>;
using std::experimental::operator>=;

using std::experimental::operator<<;

}  // namespace stdx
}  // namespace v1
}  // namespace mlio

#endif
