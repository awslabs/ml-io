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
#include <optional>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/record_reader_base.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup records Records
/// @{

/// Represents an abstract helper base class for reading records from an
/// @ref Input_stream.
class MLIO_API Stream_record_reader : public Record_reader_base {
public:
    Stream_record_reader(const Stream_record_reader &) = delete;

    Stream_record_reader &operator=(const Stream_record_reader &) = delete;

    Stream_record_reader(Stream_record_reader &&) = delete;

    Stream_record_reader &operator=(Stream_record_reader &&) = delete;

    ~Stream_record_reader() override;

    /// Gets the expected size of records read from the underlying @ref
    /// Input_stream.
    std::size_t record_size_hint() const noexcept;

    /// @remark
    ///     Helps the @ref Stream_record_reader to optimize how much it
    ///     should read-ahead from the underlying @ref Input_stream.
    void set_record_size_hint(std::size_t value) noexcept;

protected:
    explicit Stream_record_reader(Intrusive_ptr<Input_stream> stream);

private:
    MLIO_HIDDEN
    std::optional<Record> read_record_core() final;

    /// When implemented in a derived class, tries to decode a Record
    /// from the specified chunk.
    ///
    /// @param chunk
    ///     A memory slice that contains one or more records.
    /// @param ignore_leftover
    ///     A boolean value indicating whether to ignore any leftover
    ///     bits than cannot be interpreted as a Record. If false, the
    ///     reader should throw an exception with a descriptive message.
    virtual std::optional<Record> decode_record(Memory_slice &chunk, bool ignore_leftover) = 0;

    std::unique_ptr<detail::Chunk_reader> chunk_reader_;
    Memory_slice chunk_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
