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

#include <optional>  // IWYU pragma: export

namespace mlio {
inline namespace v1 {
namespace stdx {

using std::optional;
using std::hash;
using std::nullopt_t;
using std::bad_optional_access;

using std::nullopt;

using std::in_place;
using std::in_place_t;

using std::operator==;
using std::operator!=;
using std::operator<;
using std::operator<=;
using std::operator>;
using std::operator>=;

using std::make_optional;
using std::swap;

}  // namespace stdx
}  // namespace v1
}  // namespace mlio

#else

#include <experimental/optional>  // IWYU pragma: export

namespace mlio {
inline namespace v1 {
namespace stdx {

using std::experimental::optional;
using std::hash;
using std::experimental::nullopt_t;
using std::experimental::bad_optional_access;

using std::experimental::nullopt;

using std::experimental::in_place;
using std::experimental::in_place_t;

using std::experimental::operator==;
using std::experimental::operator!=;
using std::experimental::operator<;
using std::experimental::operator<=;
using std::experimental::operator>;
using std::experimental::operator>=;

using std::experimental::make_optional;
using std::experimental::swap;

}  // namespace stdx
}  // namespace v1
}  // namespace mlio

#endif
