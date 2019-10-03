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

#include "mlio/data_stores/detail/file_util.h"

#include <cstddef>

namespace mlio {
inline namespace v1 {
namespace detail {

compression
infer_compression(stdx::string_view pathname) noexcept
{
    std::size_t len = pathname.size();

    if (len > 3 && pathname[len - 3] == '.') {
        if (pathname[len - 2] == 'g' &&
            pathname[len - 1] == 'z') {

            return compression::gzip;
        }
        return compression::none;
    }

    if (len > 4 && pathname[len - 4] == '.') {
        if (pathname[len - 3] == 'b' &&
            pathname[len - 2] == 'z' &&
            pathname[len - 1] == '2') {

            return compression::bzip2;
        }

        if (pathname[len - 3] == 'z' &&
            pathname[len - 2] == 'i' &&
            pathname[len - 1] == 'p') {

            return compression::zip;
        }
    }

    return compression::none;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
