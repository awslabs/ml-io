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
inline namespace v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Represents an S3 object as a @ref data_store.
class MLIO_API s3_object final : public data_store {
public:
    /// @param cmp
    ///     The compression type of the S3 object. If set to @c infer,
    ///     the compression will be inferred from the URI.
    explicit s3_object(intrusive_ptr<s3_client const> client,
                       std::string uri,
                       std::string version_id = {},
                       compression cmp = compression::infer);

public:
    intrusive_ptr<input_stream> open_read() const final;

    std::string repr() const final;

public:
    std::string const &id() const final;

private:
    intrusive_ptr<s3_client const> client_;
    std::string uri_;
    std::string version_id_;
    compression compression_;
    mutable std::string id_{};
};

struct MLIO_API list_s3_objects_params {
    using predicate_callback = std::function<bool(std::string const &)>;

    /// The S3 client to use.
    s3_client const *client{};
    /// The list of URIs to traverse.
    stdx::span<std::string const> uris{};
    /// The pattern to match the S3 objects against.
    std::string const *pattern{};
    /// The callback function for user-specific filtering.
    predicate_callback const *predicate{};
    /// The compression type of the S3 objects. If set to @c infer, the
    /// compression will be inferred from the URIs.
    compression cmp = compression::infer;
};

/// Lists all S3 objects residing under the specified URIs.
MLIO_API
std::vector<intrusive_ptr<data_store>> list_s3_objects(list_s3_objects_params const &prm);

MLIO_API
std::vector<intrusive_ptr<data_store>>
list_s3_objects(s3_client const &client, std::string const &uri, std::string const &pattern = {});

/// @}

}  // namespace v1
}  // namespace mlio
