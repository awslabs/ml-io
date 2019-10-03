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

#include "mlio/data_stores/file_hierarchy.h"

#include "mlio/data_stores/data_store.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace v1 {

std::vector<intrusive_ptr<data_store>>
list_files(std::string const &pathname, std::string const &pattern)
{
    stdx::span<std::string const> pathnames{&pathname, 1};

    return list_files({pathnames, &pattern});
}

}  // namespace v1
}  // namespace mlio
