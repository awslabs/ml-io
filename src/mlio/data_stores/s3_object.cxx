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

#include "mlio/data_stores/s3_object.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fnmatch.h>

#include "mlio/data_stores/detail/file_util.h"
#include "mlio/detail/s3_utils.h"
#include "mlio/logger.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/s3_input_stream.h"

namespace mlio {
inline namespace v1 {

s3_object::s3_object(intrusive_ptr<s3_client const> client,
                     std::string uri,
                     std::string version_id,
                     compression cmp)
    : client_{std::move(client)}
    , uri_{std::move(uri)}
    , version_id_{std::move(version_id)}
    , compression_{cmp}
{
    detail::validate_s3_uri(uri_);

    if (compression_ == compression::infer) {
        compression_ = detail::infer_compression(uri_);
    }
}

intrusive_ptr<input_stream>
s3_object::open_read() const
{
    if (logger::is_enabled_for(log_level::info)) {
        logger::info("The S3 object '{0}' is being opened.", id());
    }

    intrusive_ptr<input_stream> strm =
        make_s3_input_stream(client_, uri_, version_id_);

    if (compression_ == compression::none) {
        return strm;
    }
    return make_inflate_stream(std::move(strm), compression_);
}

std::string const &
s3_object::id() const
{
    if (id_.empty()) {
        if (version_id_.empty()) {
            return uri_;
        }

        id_ = uri_ + "@" + version_id_;
    }

    return id_;
}

std::string
s3_object::repr() const
{
    return fmt::format("<s3_object uri='{0}' version='{1}' compression='{2}'>",
                       uri_,
                       version_id_,
                       compression_);
}

namespace detail {
namespace {

void
list_s3_objects(list_s3_objects_params const &prm,
                std::string const &uri,
                std::vector<std::string> &object_uris)
{
    auto [bucket, prefix] = detail::split_s3_uri_to_bucket_and_key(uri);

    prm.client->list_objects(bucket, prefix, [&](std::string object_uri) {
        // Pattern match.
        std::string const *pattern = prm.pattern;
        if (pattern != nullptr && !pattern->empty()) {
            int r = ::fnmatch(pattern->c_str(), object_uri.c_str(), 0);
            if (r == FNM_NOMATCH) {
                return;
            }
            if (r != 0) {
                throw std::invalid_argument{
                    "The pattern cannot be used for comparison."};
            }
        }

        // Predicate match.
        auto const *predicate = prm.predicate;
        if (predicate != nullptr && *predicate != nullptr) {
            if (!(*predicate)(object_uri)) {
                return;
            }
        }

        object_uris.emplace_back(std::move(object_uri));
    });
}

}  // namespace
}  // namespace detail

#ifdef MLIO_PLATFORM_LINUX
#define mlio_uri_comparer ::strverscmp
#else
#define mlio_uri_comparer ::strcmp
#endif

std::vector<intrusive_ptr<data_store>>
list_s3_objects(list_s3_objects_params const &prm)
{
    std::vector<std::string> object_uris{};

    for (std::string const &uri : prm.uris) {
        detail::list_s3_objects(prm, uri, object_uris);
    }

    std::sort(object_uris.begin(),
              object_uris.end(),
              [](auto const &a, auto const &b) {
                  return mlio_uri_comparer(a.c_str(), b.c_str()) < 0;
              });

    auto clt = wrap_intrusive(prm.client);

    std::vector<intrusive_ptr<data_store>> stores{};
    stores.reserve(object_uris.size());

    for (std::string const &uri : object_uris) {
        stores.emplace_back(
            make_intrusive<s3_object>(clt, uri, std::string{}, prm.cmp));
    }

    return stores;
}

std::vector<intrusive_ptr<data_store>>
list_s3_objects(s3_client const &client,
                std::string const &uri,
                std::string const &pattern)
{
    stdx::span<std::string const> uris{&uri, 1};

    return list_s3_objects({&client, uris, &pattern});
}

}  // namespace v1
}  // namespace mlio
