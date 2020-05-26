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

#include "mlio/util/string.h"

#include <algorithm>
#include <cctype>
#include <iterator>

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

template<typename It>
inline auto find_first_non_space(It first, It last)
{
    auto pos = std::find_if_not(first, last, [](int ch) {
        return std::isspace(ch);
    });

    return static_cast<std::string_view::size_type>(pos - first);
}

inline void ltrim(std::string_view &s) noexcept
{
    auto offset = find_first_non_space(s.cbegin(), s.cend());

    s.remove_prefix(offset);
}

inline void rtrim(std::string_view &s) noexcept
{
    auto offset = find_first_non_space(s.crbegin(), s.crend());

    s.remove_suffix(offset);
}

}  // namespace
}  // namespace detail

std::string_view trim(std::string_view s) noexcept
{
    detail::ltrim(s);
    detail::rtrim(s);

    return s;
}

bool is_whitespace_only(std::string_view s) noexcept
{
    auto pos = std::find_if_not(s.cbegin(), s.cend(), [](int ch) {
        return std::isspace(ch);
    });

    return pos == s.cend();
}

}  // namespace abi_v1
}  // namespace mlio
