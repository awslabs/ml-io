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

#include "mlio/data_stores/detail/util.h"

#include <cstddef>

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Compression infer_compression(std::string_view path) noexcept
{
    std::size_t len = path.size();

    if (len > 3 && path[len - 3] == '.') {
        if (path[len - 2] == 'g' && path[len - 1] == 'z') {
            return Compression::gzip;
        }
        return Compression::none;
    }

    if (len > 4 && path[len - 4] == '.') {
        if (path[len - 3] == 'b' && path[len - 2] == 'z' && path[len - 1] == '2') {
            return Compression::bzip2;
        }

        if (path[len - 3] == 'z' && path[len - 2] == 'i' && path[len - 1] == 'p') {
            return Compression::zip;
        }
    }

    return Compression::none;
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
