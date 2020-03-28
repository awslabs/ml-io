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

#include <cstddef>
#include <string>

#include "mlio/config.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/s3_client.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream_base.h"

namespace mlio {
inline namespace v1 {
namespace detail {

struct s3_input_stream_access;

}  // namespace detail

/// @addtogroup streams Streams
/// @{

class MLIO_API s3_input_stream final : public input_stream_base {
    friend struct detail::s3_input_stream_access;

private:
    explicit s3_input_stream(intrusive_ptr<s3_client const> client,
                             std::string bucket,
                             std::string key,
                             std::string version_id);

public:
    using input_stream_base::read;

    std::size_t
    read(mutable_memory_span dest) final;

    void
    seek(std::size_t position) final;

    void
    close() noexcept final;

private:
    MLIO_HIDDEN
    void
    fetch_size();

    MLIO_HIDDEN
    void
    check_if_closed() const;

public:
    std::size_t
    size() const final
    {
        return size_;
    }

    std::size_t
    position() const final
    {
        return position_;
    }

    bool
    closed() const noexcept final
    {
        return closed_;
    }

    bool
    seekable() const noexcept final
    {
        return true;
    }

private:
    intrusive_ptr<s3_client const> client_;
    std::string bucket_;
    std::string key_;
    std::string version_id_;
    bool closed_{};
    std::size_t size_{};
    std::size_t position_{};
};

MLIO_API
intrusive_ptr<s3_input_stream>
make_s3_input_stream(intrusive_ptr<s3_client const> client,
                     std::string const &uri,
                     std::string version_id = {});

/// @}

}  // namespace v1
}  // namespace mlio
