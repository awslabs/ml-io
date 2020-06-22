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

#include <string_view>
#include <utility>

namespace mlio {
inline namespace abi_v1 {
namespace detail {

std::pair<std::string_view, std::string_view>
split_s3_uri_to_bucket_and_prefix(std::string_view uri);

std::pair<std::string_view, std::string_view> split_s3_uri_to_bucket_and_key(std::string_view uri);

void validate_s3_object_uri(std::string_view uri);

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
