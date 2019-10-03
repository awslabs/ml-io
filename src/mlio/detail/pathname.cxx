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

#include "mlio/detail/pathname.h"

#include <stdexcept>

namespace mlio {
inline namespace v1 {
namespace detail {

void
validate_file_pathname(stdx::string_view pathname)
{
    if (pathname.empty()) {
        throw std::invalid_argument{"The pathname cannot be an empty string."};
    }

    if (pathname.back() == '/' || pathname.back() == '\\') {
        throw std::invalid_argument{
            "The pathname cannot point to a directory."};
    }
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
