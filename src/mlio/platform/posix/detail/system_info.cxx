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

#include "mlio/detail/system_info.h"

#include "mlio/config.h"

#if defined(MLIO_PLATFORM_LINUX)

// IWYU pragma: no_include <linux/sysinfo.h>

#include <sys/sysinfo.h>

namespace mlio {
inline namespace v1 {
namespace detail {

std::size_t
get_total_ram() noexcept
{
    struct ::sysinfo info{};
    if (::sysinfo(&info) == -1) {
        return 0;
    }
    return info.totalram;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio

#elif defined(MLIO_PLATFORM_MACOS)

#include <array>

#include <sys/sysctl.h>
#include <sys/types.h>

namespace mlio {
inline namespace v1 {
namespace detail {

std::size_t
get_total_ram() noexcept
{
    std::array<int, 2> name{CTL_HW, HW_MEMSIZE};

    std::size_t data;
    std::size_t size;

    if (::sysctl(name.data(), name.size(), &data, &size, nullptr, 0) == -1) {
        return 0;
    }

    return data;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio

#endif
