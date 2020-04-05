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
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/span.h"

namespace Aws::S3 {

class S3Client;

}  // namespace Aws::S3

namespace mlio {
inline namespace v1 {

/// Represents a client to access Amazon S3.
class MLIO_API s3_client : public intrusive_ref_counter<s3_client> {
public:
    explicit s3_client();

    explicit s3_client(std::unique_ptr<Aws::S3::S3Client> clt) noexcept;

    s3_client(s3_client const &) = delete;

    s3_client(s3_client &&) = delete;

    ~s3_client();

public:
    s3_client &
    operator=(s3_client const &) = delete;

    s3_client &
    operator=(s3_client &&) = delete;

public:
    void
    list_objects(std::string_view bucket,
                 std::string_view prefix,
                 std::function<void(std::string uri)> const &callback) const;

    std::size_t
    read_object(std::string_view bucket,
                std::string_view key,
                std::string_view version_id,
                std::size_t offset,
                mutable_memory_span dest) const;

    std::size_t
    read_object_size(std::string_view bucket,
                     std::string_view key,
                     std::string_view version_id) const;

private:
    std::unique_ptr<Aws::S3::S3Client> core_;
};

class MLIO_API s3_client_builder {
public:
    s3_client_builder &
    with_access_key_id(std::string value) noexcept
    {
        access_key_id_ = std::move(value);

        return *this;
    }

    s3_client_builder &
    with_secret_key(std::string value) noexcept
    {
        secret_key_ = std::move(value);

        return *this;
    }

    s3_client_builder &
    with_session_token(std::string value) noexcept
    {
        session_token_ = std::move(value);

        return *this;
    }

    s3_client_builder &
    with_profile(std::string value) noexcept
    {
        profile_ = std::move(value);

        return *this;
    }

    s3_client_builder &
    with_region(std::string value) noexcept
    {
        region_ = std::move(value);

        return *this;
    }

    s3_client_builder &
    with_https(bool value) noexcept
    {
        https_ = value;

        return *this;
    }

    intrusive_ptr<s3_client>
    build();

private:
    std::string access_key_id_{};
    std::string secret_key_{};
    std::string session_token_{};
    std::string profile_{};
    std::string region_{};
    bool https_{true};
};

}  // namespace v1
}  // namespace mlio
