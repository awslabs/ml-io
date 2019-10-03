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
#include <cstdint>
#include <utility>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/optional.h"
#include "mlio/record_readers/stream_record_reader.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup records Records
/// @{

namespace detail {

/// Represents a @ref record_reader for reading Parquet records.
class MLIO_API parquet_record_reader final : public stream_record_reader {
private:
    static constexpr std::size_t magic_number_size_ = sizeof(std::uint32_t);

public:
    explicit
    parquet_record_reader(intrusive_ptr<input_stream> strm)
        : stream_record_reader{std::move(strm)}
    {}

private:
    MLIO_HIDDEN
    stdx::optional<record>
    decode_record(memory_slice &chunk, bool ignore_leftover) final;

    MLIO_HIDDEN
    static bool
    is_magic_number(memory_block::iterator pos) noexcept;

    MLIO_HIDDEN
    static bool
    is_footer(memory_slice const &chunk, memory_block::iterator pos) noexcept;

    MLIO_HIDDEN
    static bool
    is_file_metadata_begin(memory_block::iterator pos) noexcept;

    template<typename T>
    MLIO_HIDDEN
    static T
    as(memory_block::iterator pos) noexcept;
};

}  // namespace detail

/// @}

}  // namespace v1
}  // namespace mlio
