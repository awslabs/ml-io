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

#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/detail/chunk_reader.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class default_chunk_reader : public chunk_reader {
public:
    explicit
    default_chunk_reader(intrusive_ptr<input_stream> strm) noexcept
        : stream_{std::move(strm)}
    {}

public:
    memory_slice
    read_chunk(memory_span leftover) final;

public:
    bool
    eof() const noexcept final
    {
        return eof_;
    }

    std::size_t
    chunk_size_hint() const noexcept final
    {
        return next_chunk_size_;
    }

    void
    set_chunk_size_hint(std::size_t value) noexcept final;

private:
    intrusive_ptr<input_stream> stream_;
    std::size_t next_chunk_size_ = 0x200'0000;  // 32 MiB
    intrusive_ptr<mutable_memory_block> chunk_{};
    bool eof_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
