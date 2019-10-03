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

#include "mlio/config.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup streams Streams
/// @{

/// Represents an input stream of bytes.
class MLIO_API input_stream : public intrusive_ref_counter<input_stream> {
public:
    input_stream() noexcept = default;

    input_stream(input_stream const &) = delete;

    input_stream(input_stream &&) = delete;

    virtual
   ~input_stream();

public:
    input_stream &
    operator=(input_stream const &) = delete;

    input_stream &
    operator=(input_stream &&) = delete;

public:
    virtual std::size_t
    read(mutable_memory_span dest) = 0;

    virtual memory_slice
    read(std::size_t size) = 0;

    virtual void
    seek(std::size_t position) = 0;

    virtual void
    close() noexcept = 0;

public:
    virtual std::size_t
    size() const = 0;

    virtual std::size_t
    position() const = 0;

    virtual bool
    closed() const noexcept = 0;

    virtual bool
    seekable() const noexcept = 0;

    virtual bool
    supports_zero_copy() const noexcept = 0;
};

/// @}

}  // namespace v1
}  // namespace mlio
