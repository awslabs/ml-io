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
inline namespace abi_v1 {

/// Represents a client to access Amazon S3.
class MLIO_API S3_client : public Intrusive_ref_counter<S3_client> {
public:
    explicit S3_client(std::unique_ptr<Aws::S3::S3Client> native_client) noexcept;

    S3_client(const S3_client &) = delete;

    S3_client &operator=(const S3_client &) = delete;

    S3_client(S3_client &&) = delete;

    S3_client &operator=(S3_client &&) = delete;

    ~S3_client();

    void list_objects(std::string_view bucket,
                      std::string_view prefix,
                      const std::function<void(std::string uri)> &callback) const;

    std::size_t read_object(std::string_view bucket,
                            std::string_view key,
                            std::string_view version_id,
                            std::size_t offset,
                            Mutable_memory_span destination) const;

    std::size_t read_object_size(std::string_view bucket,
                                 std::string_view key,
                                 std::string_view version_id) const;

private:
    std::unique_ptr<Aws::S3::S3Client> native_client_;
};

struct MLIO_API S3_client_options {
    std::string_view access_key_id{};
    std::string_view secret_key{};
    std::string_view session_token{};
    std::string_view profile{};
    std::string_view region{};
    bool use_https{true};
};

MLIO_API
Intrusive_ptr<S3_client> make_s3_client(const S3_client_options &opts = {});

}  // namespace abi_v1
}  // namespace mlio
