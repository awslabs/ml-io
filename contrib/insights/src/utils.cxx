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

#include "utils.h"
#include "mlio.h"

namespace insights {
namespace details {

bool
match_nan_values(std::string_view const s,
                 const std::vector<std::string> &match_values) noexcept
{
    std::string_view trimmed = mlio::trim(s);
    for (const std::string &needle : match_values) {
        auto matches = std::equal(
            needle.begin(),
            needle.end(),
            trimmed.begin(),
            trimmed.end(),
            [](const char &c1, const char &c2) {
                return (c1 == c2 || std::tolower(c1) == std::tolower(c2));
            });
        if (matches) {
            return true;
        }
    }
    return false;
}

}  // namespace details
}  // namespace insights
