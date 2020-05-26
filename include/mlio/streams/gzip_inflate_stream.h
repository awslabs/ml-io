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
inline namespace abi_v1 {

/// @addtogroup streams Streams
/// @{

/// Represents an @ref Input_stream that inflates an underlying
/// stream that was deflated with gzip or zlib.
class MLIO_API Gzip_inflate_stream final : public Input_stream_base {
public:
    explicit Gzip_inflate_stream(Intrusive_ptr<Input_stream> inner);

    Gzip_inflate_stream(const Gzip_inflate_stream &) = delete;

    Gzip_inflate_stream &operator=(const Gzip_inflate_stream &) = delete;

    Gzip_inflate_stream(Gzip_inflate_stream &&) = delete;

    Gzip_inflate_stream &operator=(Gzip_inflate_stream &&) = delete;

    ~Gzip_inflate_stream() final;

    using Input_stream_base::read;

    std::size_t read(Mutable_memory_span destination) final;

    void close() noexcept final;

    bool closed() const noexcept final;

private:
    MLIO_HIDDEN
    void check_if_closed() const;

    Intrusive_ptr<Input_stream> inner_;
    std::unique_ptr<detail::Zlib_inflater> inflater_;
    Memory_slice buffer_{};
    Memory_block::iterator buffer_pos_ = buffer_.begin();
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
