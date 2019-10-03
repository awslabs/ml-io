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
#include <memory>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream_base.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup streams Streams
/// @{

/// Represents an @ref input_stream that inflates an underlying
/// stream that was deflated with gzip or zlib.
class MLIO_API gzip_inflate_stream final : public input_stream_base {
public:
    explicit
    gzip_inflate_stream(intrusive_ptr<input_stream> inner);

    gzip_inflate_stream(gzip_inflate_stream const &) = delete;

    gzip_inflate_stream(gzip_inflate_stream &&) = delete;

   ~gzip_inflate_stream() final;

public:
    gzip_inflate_stream &
    operator=(gzip_inflate_stream const &) = delete;

    gzip_inflate_stream &
    operator=(gzip_inflate_stream &&) = delete;

public:
    using input_stream_base::read;

    std::size_t
    read(mutable_memory_span dest) final;

    void
    close() noexcept final;

private:
    MLIO_HIDDEN
    void
    check_if_closed() const;

public:
    bool
    closed() const noexcept final;

private:
    intrusive_ptr<input_stream> inner_;
    std::unique_ptr<detail::zlib_inflater> inflater_;
    memory_slice buffer_{};
    memory_block::iterator buffer_pos_ = buffer_.begin();
};

/// @}

}  // namespace v1
}  // namespace mlio
