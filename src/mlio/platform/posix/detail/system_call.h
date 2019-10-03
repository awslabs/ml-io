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

#include <utility>

namespace mlio {
inline namespace v1 {
namespace detail {


// C++ equivalent of the TEMP_FAILURE_RETRY macro.
template<typename Func, typename... Args>
inline int
temp_failure_retry(Func syscall, Args &&...args) noexcept
{
    int r;

    do {
        r = syscall(std::forward<Args>(args)...);
    } while (r == -1 && errno == EINTR);

    return r;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
