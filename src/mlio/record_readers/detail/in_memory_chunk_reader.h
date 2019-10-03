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

#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/detail/chunk_reader.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class in_memory_chunk_reader : public chunk_reader {
public:
    explicit
    in_memory_chunk_reader(memory_slice &&chunk) noexcept
        : chunk_{std::move(chunk)}
    {}

public:
    memory_slice
    read_chunk(memory_span leftover) final;

public:
    bool
    eof() const noexcept final
    {
        return chunk_.empty();
    }

    std::size_t
    chunk_size_hint() const noexcept final
    {
        return 0;
    }

    void
    set_chunk_size_hint(std::size_t) noexcept final
    {}

private:
    memory_slice chunk_;
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
