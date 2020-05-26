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
#include "mlio/intrusive_ptr.h"
#include "mlio/s3_client.h"
#include "mlio/span.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Represents an S3 object as a @ref Data_store.
class MLIO_API S3_object final : public Data_store {
public:
    /// @param compression
    ///     The Compression type of the S3 object. If set to @c infer,
    ///     the Compression will be inferred from the URI.
    explicit S3_object(Intrusive_ptr<const S3_client> client,
                       std::string uri,
                       std::string version_id = {},
                       Compression compression = Compression::infer);

    Intrusive_ptr<Input_stream> open_read() const final;

    std::string repr() const final;

    const std::string &id() const final;

private:
    Intrusive_ptr<const S3_client> client_;
    std::string uri_;
    std::string version_id_;
    Compression compression_;
    mutable std::string id_{};
};

struct MLIO_API List_s3_objects_params {
    using predicate_callback = std::function<bool(const std::string &)>;

    /// The S3 client to use.
    const S3_client *client{};
    /// The list of URIs to traverse.
    stdx::span<const std::string> uris{};
    /// The pattern to match the S3 objects against.
    const std::string *pattern{};
    /// The callback function for user-specific filtering.
    const predicate_callback *predicate{};
    /// The Compression type of the S3 objects. If set to @c infer, the
    /// Compression will be inferred from the URIs.
    Compression compression = Compression::infer;
};

/// Lists all S3 objects residing under the specified URIs.
MLIO_API
std::vector<Intrusive_ptr<Data_store>> list_s3_objects(const List_s3_objects_params &params);

MLIO_API
std::vector<Intrusive_ptr<Data_store>>
list_s3_objects(const S3_client &client, const std::string &uri, const std::string &pattern = {});

/// @}

}  // namespace abi_v1
}  // namespace mlio
