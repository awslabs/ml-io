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

#include "mlio/streams/s3_input_stream.h"

#include <algorithm>
#include <system_error>
#include <utility>

#include "mlio/detail/s3_utils.h"
#include "mlio/streams/stream_error.h"

namespace mlio {
inline namespace abi_v1 {

std::size_t S3_input_stream::read(Mutable_memory_span destination)
{
    check_if_closed();

    if (destination.empty() || position_ == size_) {
        return 0;
    }

    destination = destination.first(std::min(size_ - position_, destination.size()));

    auto num_bytes_read = client_->read_object(bucket_, key_, version_id_, position_, destination);

    position_ += num_bytes_read;

    return num_bytes_read;
}

void S3_input_stream::seek(std::size_t position)
{
    check_if_closed();

    if (position > size_) {
        throw std::system_error{std::make_error_code(std::errc::invalid_argument)};
    }

    position_ = position;
}

void S3_input_stream::close() noexcept
{
    closed_ = true;
}

S3_input_stream::S3_input_stream(Intrusive_ptr<const S3_client> client,
                                 std::string bucket,
                                 std::string key,
                                 std::string version_id)
    : client_{std::move(client)}
    , bucket_{std::move(bucket)}
    , key_{std::move(key)}
    , version_id_{std::move(version_id)}
{}

void S3_input_stream::fetch_size()
{
    size_ = client_->read_object_size(bucket_, key_, version_id_);
}

void S3_input_stream::check_if_closed() const
{
    if (closed_) {
        throw Stream_error{"The input stream is closed."};
    }
}

namespace detail {

struct S3_input_stream_access {
    static inline Intrusive_ptr<S3_input_stream>
    make(Intrusive_ptr<const S3_client> client, const std::string &uri, std::string version_id)
    {
        auto [bucket, key] = split_s3_uri_to_bucket_and_key(uri);

        auto *ptr = new S3_input_stream{
            std::move(client), std::string{bucket}, std::string{key}, std::move(version_id)};

        auto stream = wrap_intrusive(ptr);

        stream->fetch_size();

        return stream;
    }
};

}  // namespace detail

Intrusive_ptr<S3_input_stream> make_s3_input_stream(Intrusive_ptr<const S3_client> client,
                                                    const std::string &uri,
                                                    std::string version_id)
{
    return detail::S3_input_stream_access::make(std::move(client), uri, std::move(version_id));
}

}  // namespace abi_v1
}  // namespace mlio
