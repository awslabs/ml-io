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

#include "mlio/detail/path.h"

#include <stdexcept>

namespace mlio {
inline namespace abi_v1 {
namespace detail {

void validate_file_path(std::string_view path)
{
    if (path.empty()) {
        throw std::invalid_argument{"The path cannot be an empty string."};
    }

    if (path.back() == '/' || path.back() == '\\') {
        throw std::invalid_argument{"The path cannot point to a directory."};
    }
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
