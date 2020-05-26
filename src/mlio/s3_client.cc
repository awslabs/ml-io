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

#include "mlio/s3_client.h"

#ifdef MLIO_BUILD_S3

#include <mutex>
#include <system_error>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/core/auth/AWSCredentialsProviderChain.h>
#include <aws/core/client/AWSError.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/S3Errors.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>

#include "mlio/util/cast.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

std::once_flag chain_initialized;

Aws::Auth::AWSCredentials get_default_aws_credentials()
{
    std::unique_ptr<Aws::Auth::DefaultAWSCredentialsProviderChain> chain{};

    // Compilers are allowed to initialize local static variables before
    // entering the main function. As the credentials provider chain has
    // to be constructed after calling the Aws::InitAPI function here we
    // initialize lazily.
    std::call_once(chain_initialized, [&chain]() {
        chain = std::make_unique<Aws::Auth::DefaultAWSCredentialsProviderChain>();
    });

    return chain->GetAWSCredentials();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

[[noreturn]] void throw_s3_error(const Aws::Client::AWSError<Aws::S3::S3Errors> &err)
{
    std::error_code ec;

    switch (err.GetErrorType()) {
    case Aws::S3::S3Errors::SERVICE_UNAVAILABLE:
        ec = std::make_error_code(std::errc::host_unreachable);
        break;
    case Aws::S3::S3Errors::ACCESS_DENIED:
        ec = std::make_error_code(std::errc::permission_denied);
        break;
    case Aws::S3::S3Errors::RESOURCE_NOT_FOUND:
    case Aws::S3::S3Errors::NO_SUCH_BUCKET:
    case Aws::S3::S3Errors::NO_SUCH_KEY:
        ec = std::make_error_code(std::errc::no_such_file_or_directory);
        break;
    case Aws::S3::S3Errors::REQUEST_TIMEOUT:
        ec = std::make_error_code(std::errc::timed_out);
        break;
    default:
        ec = std::make_error_code(std::errc::io_error);
        break;
    }

    throw std::system_error{ec, "The S3 object cannot be accessed."};
}

#pragma GCC diagnostic pop

template<typename Outcome>
inline void check_s3_error(const Outcome &outcome)
{
    if (!outcome.IsSuccess()) {
        throw_s3_error(outcome.GetError());
    }
}

}  // namespace
}  // namespace detail

S3_client::S3_client() : native_client_{}
{
    Aws::Auth::AWSCredentials credentials = detail::get_default_aws_credentials();

    native_client_ = std::make_unique<Aws::S3::S3Client>(credentials);
}

S3_client::S3_client(std::unique_ptr<Aws::S3::S3Client> native_client) noexcept
    : native_client_{std::move(native_client)}
{}

S3_client::~S3_client() = default;

void S3_client::list_objects(std::string_view bucket,
                             std::string_view prefix,
                             const std::function<void(std::string uri)> &callback) const
{
    Aws::S3::Model::ListObjectsV2Request request{};
    request.SetBucket(Aws::String{bucket});
    request.SetMaxKeys(1000);

    if (!prefix.empty()) {
        request.SetPrefix(Aws::String{prefix});
    }

    std::string base_uri{"s3://" + std::string{bucket} + "/"};

    while (true) {
        auto outcome = native_client_->ListObjectsV2(request);
        detail::check_s3_error(outcome);

        const auto &result = outcome.GetResult();

        for (const auto &obj : result.GetContents()) {
            callback(base_uri + std::string{obj.GetKey()});
        }

        if (!result.GetIsTruncated()) {
            break;
        }
        request.SetContinuationToken(result.GetNextContinuationToken());
    }
}

std::size_t S3_client::read_object(std::string_view bucket,
                                   std::string_view key,
                                   std::string_view version_id,
                                   std::size_t offset,
                                   Mutable_memory_span destination) const
{
    std::string range_str =
        "bytes=" + std::to_string(offset) + "-" + std::to_string(offset + destination.size());

    Aws::S3::Model::GetObjectRequest request{};
    request.SetBucket(Aws::String{bucket});
    request.SetKey(Aws::String{key});
    request.SetRange(Aws::String{range_str});

    if (!version_id.empty()) {
        request.SetVersionId(Aws::String{version_id});
    }

    auto outcome = native_client_->GetObject(request);
    detail::check_s3_error(outcome);

    auto chars = as_span<char>(destination);

    auto &body = outcome.GetResult().GetBody();
    body.read(chars.data(), static_cast<std::streamsize>(chars.size()));

    return static_cast<std::size_t>(body.gcount()) * sizeof(char);
}

std::size_t S3_client::read_object_size(std::string_view bucket,
                                        std::string_view key,
                                        std::string_view version_id) const
{
    Aws::S3::Model::HeadObjectRequest request{};
    request.SetBucket(Aws::String{bucket});
    request.SetKey(Aws::String{key});

    if (!version_id.empty()) {
        request.SetVersionId(Aws::String{version_id});
    }

    auto outcome = native_client_->HeadObject(request);
    detail::check_s3_error(outcome);

    return as_size(outcome.GetResult().GetContentLength());
}

Intrusive_ptr<S3_client> S3_client_builder::build()
{
    Aws::Auth::AWSCredentials credentials{};
    if (access_key_id_.empty() && secret_key_.empty()) {
        credentials = detail::get_default_aws_credentials();
    }
    else {
        credentials = Aws::Auth::AWSCredentials{
            Aws::String{access_key_id_}, Aws::String{secret_key_}, Aws::String{session_token_}};
    }

    Aws::Client::ClientConfiguration config{};
    if (!profile_.empty()) {
        config = Aws::Client::ClientConfiguration{profile_.c_str()};
    }
    if (!region_.empty()) {
        config.region = region_;
    }
    if (!use_https_) {
        config.scheme = Aws::Http::Scheme::HTTP;
    }

    auto native_client = std::make_unique<Aws::S3::S3Client>(credentials, config);

    return make_intrusive<S3_client>(std::move(native_client));
}

}  // namespace abi_v1
}  // namespace mlio

#else

#include "mlio/Not_supported_error.h"

namespace Aws::S3 {

class S3Client {};

}  // namespace Aws::S3

namespace mlio {
inline namespace abi_v1 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"

S3_client::S3_client() : native_client_{}
{
    throw Not_supported_error{"MLIO was not built with S3 support."};
}

S3_client::S3_client(std::unique_ptr<Aws::S3::S3Client>) noexcept : native_client_{}
{}

S3_client::~S3_client() = default;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void S3_client::list_objects(std::string_view,
                             std::string_view,
                             const std::function<void(std::string)> &) const
{}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::size_t S3_client::read_object(std::string_view,
                                   std::string_view,
                                   std::string_view,
                                   std::size_t,
                                   Mutable_memory_span) const
{
    return 0;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::size_t S3_client::read_object_size(std::string_view, std::string_view, std::string_view) const
{
    return 0;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
Intrusive_ptr<S3_client> S3_client_builder::build()
{
    throw Not_supported_error{"MLIO was not built with S3 support."};
}

#pragma GCC diagnostic pop

}  // namespace abi_v1
}  // namespace mlio

#endif
