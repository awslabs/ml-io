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

#include "mlio/config.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/span.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup streams Streams
/// @{

/// Represents an input stream of bytes.
class MLIO_API Input_stream : public Intrusive_ref_counter<Input_stream> {
public:
    Input_stream() noexcept = default;

    Input_stream(const Input_stream &) = delete;

    Input_stream &operator=(const Input_stream &) = delete;

    Input_stream(Input_stream &&) = delete;

    Input_stream &operator=(Input_stream &&) = delete;

    virtual ~Input_stream();

    virtual std::size_t read(Mutable_memory_span destination) = 0;

    virtual Memory_slice read(std::size_t size) = 0;

    virtual void seek(std::size_t position) = 0;

    virtual void close() noexcept = 0;

    virtual std::size_t size() const = 0;

    virtual std::size_t position() const = 0;

    virtual bool closed() const noexcept = 0;

    virtual bool seekable() const noexcept = 0;

    virtual bool supports_zero_copy() const noexcept = 0;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
