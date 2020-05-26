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
#include <utility>

#include "mlio/config.h"
#include "mlio/memory/memory_block.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup streams Streams
/// @{

/// Wraps a @ref Memory_slice as an @ref Input_stream.
class MLIO_API Memory_input_stream final : public Input_stream {
public:
    explicit Memory_input_stream(Memory_slice source) noexcept : source_{std::move(source)}
    {}

    std::size_t read(Mutable_memory_span destination) final;

    Memory_slice read(std::size_t size) final;

    void seek(std::size_t position) final;

    void close() noexcept final;

    std::size_t size() const final;

    std::size_t position() const final;

    bool closed() const noexcept final
    {
        return closed_;
    }

    bool seekable() const noexcept final
    {
        return true;
    }

    bool supports_zero_copy() const noexcept final
    {
        return true;
    }

private:
    MLIO_HIDDEN
    void advance_position(std::size_t distance) noexcept;

    MLIO_HIDDEN
    void check_if_closed() const;

    Memory_slice source_;
    Memory_block::iterator pos_ = source_.begin();
    bool closed_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
