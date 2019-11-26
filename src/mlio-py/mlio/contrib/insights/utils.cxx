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

#include <algorithm>
#include <cctype>

#include <mlio.h>

namespace insights {

bool
match_nan_values(std::string_view s,
                 std::vector<std::string> const &match_values) noexcept
{
    s = mlio::trim(s);

    for (auto const &needle : match_values) {
        bool matches = std::equal(needle.begin(),
                                  needle.end(),
                                  s.begin(),
                                  s.end(),
                                  [](int c1, int c2) {
                                      return c1 == c2 || std::tolower(c1) ==
                                                             std::tolower(c2);
                                  });
        if (matches) {
            return true;
        }
    }
    return false;
}

}  // namespace insights
