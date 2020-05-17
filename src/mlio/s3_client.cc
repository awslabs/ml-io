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
inline namespace v1 {
namespace detail {
namespace {

std::once_flag chain_initialized;

Aws::Auth::AWSCredentials get_default_aws_credentials()
{
    std::unique_ptr<Aws::Auth::DefaultAWSCredentialsProviderChain> chain{};

    // Compilers are allowed to initialize local static variables before
    // entering the main function. As the credentials provider chain has
    // to be constructed after Aws::InitAPI() we initialize lazily.
    std::call_once(chain_initialized, [&chain]() {
        chain = std::make_unique<Aws::Auth::DefaultAWSCredentialsProviderChain>();
    });

    return chain->GetAWSCredentials();
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"

[[noreturn]] void throw_s3_error(Aws::Client::AWSError<Aws::S3::S3Errors> const &err)
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

s3_client::s3_client() : core_{}
{
    Aws::Auth::AWSCredentials crd = detail::get_default_aws_credentials();

    core_ = std::make_unique<Aws::S3::S3Client>(crd);
}

s3_client::s3_client(std::unique_ptr<Aws::S3::S3Client> clt) noexcept : core_{std::move(clt)}
{}

s3_client::~s3_client() = default;

void s3_client::list_objects(std::string_view bucket,
                             std::string_view prefix,
                             std::function<void(std::string uri)> const &callback) const
{
    Aws::S3::Model::ListObjectsV2Request req{};
    req.SetBucket(Aws::String{bucket});
    req.SetMaxKeys(1000);

    if (!prefix.empty()) {
        req.SetPrefix(Aws::String{prefix});
    }

    std::string base_uri{"s3://" + std::string{bucket} + "/"};

    while (true) {
        auto outcome = core_->ListObjectsV2(req);
        detail::check_s3_error(outcome);

        const auto &result = outcome.GetResult();

        for (const auto &obj : result.GetContents()) {
            callback(base_uri + std::string{obj.GetKey()});
        }

        if (!result.GetIsTruncated()) {
            break;
        }
        req.SetContinuationToken(result.GetNextContinuationToken());
    }
}

std::size_t s3_client::read_object(std::string_view bucket,
                                   std::string_view key,
                                   std::string_view version_id,
                                   std::size_t offset,
                                   mutable_memory_span dest) const
{
    std::string range_str =
        "bytes=" + std::to_string(offset) + "-" + std::to_string(offset + dest.size());

    Aws::S3::Model::GetObjectRequest req{};
    req.SetBucket(Aws::String{bucket});
    req.SetKey(Aws::String{key});
    req.SetRange(Aws::String{range_str});

    if (!version_id.empty()) {
        req.SetVersionId(Aws::String{version_id});
    }

    auto outcome = core_->GetObject(req);
    detail::check_s3_error(outcome);

    auto chrs = as_span<char>(dest);

    auto &body = outcome.GetResult().GetBody();
    body.read(chrs.data(), static_cast<std::streamsize>(chrs.size()));

    return static_cast<std::size_t>(body.gcount()) * sizeof(char);
}

std::size_t s3_client::read_object_size(std::string_view bucket,
                                        std::string_view key,
                                        std::string_view version_id) const
{
    Aws::S3::Model::HeadObjectRequest req{};
    req.SetBucket(Aws::String{bucket});
    req.SetKey(Aws::String{key});

    if (!version_id.empty()) {
        req.SetVersionId(Aws::String{version_id});
    }

    auto outcome = core_->HeadObject(req);
    detail::check_s3_error(outcome);

    return as_size(outcome.GetResult().GetContentLength());
}

intrusive_ptr<s3_client> s3_client_builder::build()
{
    Aws::Auth::AWSCredentials crd{};
    if (access_key_id_.empty() && secret_key_.empty()) {
        crd = detail::get_default_aws_credentials();
    }
    else {
        crd = Aws::Auth::AWSCredentials{
            Aws::String{access_key_id_}, Aws::String{secret_key_}, Aws::String{session_token_}};
    }

    Aws::Client::ClientConfiguration cfg{};
    if (!profile_.empty()) {
        cfg = Aws::Client::ClientConfiguration{profile_.c_str()};
    }
    if (!region_.empty()) {
        cfg.region = region_;
    }
    if (!https_) {
        cfg.scheme = Aws::Http::Scheme::HTTP;
    }

    auto clt = std::make_unique<Aws::S3::S3Client>(crd, cfg);

    return make_intrusive<s3_client>(std::move(clt));
}

}  // namespace v1
}  // namespace mlio

#else

#include "mlio/not_supported_error.h"

namespace Aws::S3 {

class S3Client {};

}  // namespace Aws::S3

namespace mlio {
inline namespace v1 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"

s3_client::s3_client() : core_{}
{
    throw not_supported_error{"MLIO was not built with S3 support."};
}

s3_client::s3_client(std::unique_ptr<Aws::S3::S3Client>) noexcept : core_{}
{}

s3_client::~s3_client() = default;

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
void s3_client::list_objects(std::string_view,
                             std::string_view,
                             std::function<void(std::string)> const &) const
{}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::size_t s3_client::read_object(std::string_view,
                                   std::string_view,
                                   std::string_view,
                                   std::size_t,
                                   mutable_memory_span) const
{
    return 0;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
std::size_t s3_client::read_object_size(std::string_view, std::string_view, std::string_view) const
{
    return 0;
}

// NOLINTNEXTLINE(readability-convert-member-functions-to-static)
intrusive_ptr<s3_client> s3_client_builder::build()
{
    throw not_supported_error{"MLIO was not built with S3 support."};
}

#pragma GCC diagnostic pop

}  // namespace v1
}  // namespace mlio

#endif
