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

#include <functional>
#include <string>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_stores/compression.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_stores Data Stores
/// @{

struct MLIO_API list_files_params {
    using predicate_callback = std::function<bool(std::string const &)>;

    /// The list of pathnames to traverse.
    stdx::span<std::string const> pathnames{};
    /// The pattern to match the filenames against.
    std::string const *pattern{};
    /// The callback function for user-specific filtering.
    predicate_callback const *predicate{};
    /// A boolean value indicating whether the files should be
    /// memory-mapped.
    bool mmap = true;
    /// The compression type of the files. If set to @c infer, the
    /// compression will be inferred from the filenames.
    compression cmp = compression::infer;
};

/// Recursively list all files residing under the specified pathnames.
MLIO_API
std::vector<intrusive_ptr<data_store>>
list_files(list_files_params const &prm);

MLIO_API
std::vector<intrusive_ptr<data_store>>
list_files(std::string const &pathname, std::string const &pattern = {});

/// @}

}  // namespace v1
}  // namespace mlio
