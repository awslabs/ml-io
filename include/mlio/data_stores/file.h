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

#include <functional>
#include <string>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_stores/compression.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/span.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Represents a file as a @ref data_store.
class MLIO_API file final : public data_store {
public:
    /// @param mmap
    ///     A boolean value indicating whether the file should be
    ///     memory-mapped.
    ///
    /// @param cmp
    ///     The compression type of the file. If set to @c infer, the
    ///     compression will be inferred from the filename.
    explicit file(std::string pathname, bool mmap = true, compression cmp = compression::infer);

public:
    intrusive_ptr<input_stream> open_read() const final;

    std::string repr() const final;

public:
    const std::string &id() const noexcept final
    {
        return pathname_;
    }

private:
    std::string pathname_;
    bool mmap_;
    compression compression_;
};

struct MLIO_API list_files_params {
    using predicate_callback = std::function<bool(const std::string &)>;

    /// The list of pathnames to traverse.
    stdx::span<std::string const> pathnames{};
    /// The pattern to match the filenames against.
    const std::string *pattern{};
    /// The callback function for user-specific filtering.
    const predicate_callback *predicate{};
    /// A boolean value indicating whether the files should be
    /// memory-mapped.
    bool mmap = true;
    /// The compression type of the files. If set to @c infer, the
    /// compression will be inferred from the filenames.
    compression cmp = compression::infer;
};

/// Recursively lists all files residing under the specified pathnames.
MLIO_API
std::vector<intrusive_ptr<data_store>> list_files(const list_files_params &prm);

MLIO_API
std::vector<intrusive_ptr<data_store>>
list_files(const std::string &pathname, const std::string &pattern = {});

/// @}

}  // namespace abi_v1
}  // namespace mlio
