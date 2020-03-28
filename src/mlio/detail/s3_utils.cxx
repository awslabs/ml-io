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

#include "mlio/detail/s3_utils.h"

#include <stdexcept>

namespace mlio {
inline namespace v1 {
namespace detail {

std::pair<std::string_view, std::string_view>
split_s3_uri_to_bucket_and_key(std::string_view uri)
{
    if (uri.empty()) {
        throw std::invalid_argument{"The URI cannot be an empty string."};
    }

    if (uri.size() < 5 || uri.substr(0, 5) != "s3://") {
        throw std::invalid_argument{"The URI must start with the S3 scheme."};
    }

    uri = uri.substr(5);

    auto pos = uri.find_first_of('/');
    if (pos == std::string_view::npos) {
        throw std::invalid_argument{
            "The URI must consists of a bucket name and a key/prefix."};
    }
    if (pos == 0) {
        throw std::invalid_argument{"The URI does not contain a bucket name."};
    }
    if (pos == uri.size() - 1) {
        throw std::invalid_argument{"The URI does not contain a key/prefix."};
    }

    return std::make_pair(uri.substr(0, pos), uri.substr(pos + 1));
}

void
validate_s3_uri(std::string_view uri)
{
    split_s3_uri_to_bucket_and_key(uri);
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
