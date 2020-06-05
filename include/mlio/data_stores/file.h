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
#include <string_view>
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

/// Represents a file as a @ref Data_store.
class MLIO_API File final : public Data_store {
public:
    /// @param memory_map
    ///     A boolean value indicating whether the file should be
    ///     memory-mapped.
    ///
    /// @param compression
    ///     The compression type of the file. If set to @c infer, the
    ///     compression will be inferred from the filename.
    explicit File(std::string path,
                  bool memory_map = true,
                  Compression compression = Compression::infer);

    Intrusive_ptr<Input_stream> open_read() const final;

    std::string repr() const final;

    const std::string &id() const noexcept final
    {
        return path_;
    }

private:
    std::string path_;
    bool memory_map_;
    Compression compression_;
};

struct MLIO_API File_list_options {
    using Predicate_callback = std::function<bool(const std::string &)>;

    /// The pattern to match the filenames against.
    std::string_view pattern{};
    /// The callback function for user-specific filtering.
    const Predicate_callback *predicate{};
    /// A boolean value indicating whether the files should be
    /// memory-mapped.
    bool memory_map = true;
    /// The compression type of the files. If set to @c infer, the
    /// compression will be inferred from the filenames.
    Compression compression = Compression::infer;
};

/// Recursively lists all files residing under the specified paths.
MLIO_API
std::vector<Intrusive_ptr<Data_store>>
list_files(stdx::span<const std::string> paths, const File_list_options &opts = {});

MLIO_API
std::vector<Intrusive_ptr<Data_store>>
list_files(const std::string &path, const std::string_view pattern = {});

/// @}

}  // namespace abi_v1
}  // namespace mlio
