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

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/optional.h"
#include "mlio/record_readers/record_reader_base.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup records Records
/// @{

/// Represents an abstract helper base class for reading records from an
/// @ref input_stream.
class MLIO_API stream_record_reader : public record_reader_base {
protected:
    explicit
    stream_record_reader(intrusive_ptr<input_stream> strm);

public:
    stream_record_reader(stream_record_reader const &) = delete;

    stream_record_reader(stream_record_reader &&) = delete;

   ~stream_record_reader() override;

public:
    stream_record_reader &
    operator=(stream_record_reader const &) = delete;

    stream_record_reader &
    operator=(stream_record_reader &&) = delete;

private:
    MLIO_HIDDEN
    stdx::optional<record>
    read_record_core() final;

    /// When implemented in a derived class, tries to decode a record
    /// from the specified chunk.
    ///
    /// @param chunk
    ///     A memory slice that contains one or more records.
    /// @param ignore_leftover
    ///     A boolean value indicating whether to ignore any leftover
    ///     bits than cannot be interpreted as a record. If false, the
    ///     reader should throw an exception with a descriptive message.
    virtual stdx::optional<record>
    decode_record(memory_slice &chunk, bool ignore_leftover) = 0;

public:
    /// Gets the expected size of records read from the underlying @ref
    /// input_stream.
    std::size_t
    record_size_hint() const noexcept;

    /// @remark
    ///     Helps the @ref stream_record_reader to optimize how much it
    ///     should read-ahead from the underlying @ref input_stream.
    void
    set_record_size_hint(std::size_t value) noexcept;

private:
    std::unique_ptr<detail::chunk_reader> chunk_reader_;
    memory_slice chunk_{};
};

/// @}

}  // namespace v1
}  // namespace mlio
