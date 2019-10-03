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
#include <utility>

#include "mlio/config.h"
#include "mlio/memory/memory_block.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup streams Streams
/// @{

/// Wraps a @ref memory_slice as an @ref input_stream.
class MLIO_API memory_input_stream final : public input_stream {
public:
    explicit
    memory_input_stream(memory_slice source) noexcept
        : source_{std::move(source)}
    {}

public:
    std::size_t
    read(mutable_memory_span dest) final;

    memory_slice
    read(std::size_t size) final;

    void
    seek(std::size_t position) final;

    void
    close() noexcept final;

private:
    MLIO_HIDDEN
    void
    advance_position(std::size_t dist) noexcept;

    MLIO_HIDDEN
    void
    check_if_closed() const;

public:
    std::size_t
    size() const final;

    std::size_t
    position() const final;

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

    bool
    supports_zero_copy() const noexcept final
    {
        return true;
    }

private:
    memory_slice source_;
    memory_block::iterator pos_ = source_.begin();
    bool closed_{};
};

/// @}

}  // namespace v1
}  // namespace mlio
