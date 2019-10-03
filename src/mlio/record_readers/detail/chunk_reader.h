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

#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class chunk_reader {
public:
    chunk_reader() noexcept = default;

    chunk_reader(chunk_reader const &) = delete;

    chunk_reader(chunk_reader &&) = delete;

    virtual
   ~chunk_reader();

public:
    chunk_reader &
    operator=(chunk_reader const &) = delete;

    chunk_reader &
    operator=(chunk_reader &&) = delete;

public:
    virtual memory_slice
    read_chunk(memory_span leftover) = 0;

public:
    virtual bool
    eof() const noexcept = 0;

    virtual std::size_t
    chunk_size_hint() const noexcept = 0;

    virtual void
    set_chunk_size_hint(std::size_t value) noexcept = 0;
};

std::unique_ptr<chunk_reader>
make_chunk_reader(intrusive_ptr<input_stream> strm);

}  // namespace detail
}  // namespace v1
}  // namespace mlio
