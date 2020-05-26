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
#include <cstdint>
#include <optional>
#include <utility>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/record_readers/stream_record_reader.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup records Records
/// @{

namespace detail {

/// Represents a @ref Record_reader for reading Parquet records.
class MLIO_API Parquet_record_reader final : public Stream_record_reader {
public:
    explicit Parquet_record_reader(Intrusive_ptr<Input_stream> stream)
        : Stream_record_reader{std::move(stream)}
    {}

private:
    static constexpr std::size_t magic_number_size_ = sizeof(std::uint32_t);

    MLIO_HIDDEN
    std::optional<Record> decode_record(Memory_slice &chunk, bool ignore_leftover) final;

    MLIO_HIDDEN
    static bool is_magic_number(Memory_block::iterator pos) noexcept;

    MLIO_HIDDEN
    static bool is_footer(const Memory_slice &chunk, Memory_block::iterator pos) noexcept;

    MLIO_HIDDEN
    static bool is_file_metadata_begin(Memory_block::iterator pos) noexcept;

    template<typename T>
    MLIO_HIDDEN
    static T as(Memory_block::iterator pos) noexcept;
};

}  // namespace detail

/// @}

}  // namespace abi_v1
}  // namespace mlio
