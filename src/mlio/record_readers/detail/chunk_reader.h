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

#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/span.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Chunk_reader {
public:
    Chunk_reader() noexcept = default;

    Chunk_reader(const Chunk_reader &) = delete;

    Chunk_reader &operator=(const Chunk_reader &) = delete;

    Chunk_reader(Chunk_reader &&) = delete;

    Chunk_reader &operator=(Chunk_reader &&) = delete;

    virtual ~Chunk_reader();

    virtual Memory_slice read_chunk(Memory_span leftover) = 0;

    virtual bool eof() const noexcept = 0;

    virtual std::size_t chunk_size_hint() const noexcept = 0;

    virtual void set_chunk_size_hint(std::size_t value) noexcept = 0;
};

std::unique_ptr<Chunk_reader> make_chunk_reader(Intrusive_ptr<Input_stream> stream);

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
