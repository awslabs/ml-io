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

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/optional.h"
#include "mlio/record_readers/stream_record_reader.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup records Records
/// @{

/// Represents an abstract helper base class for reading text-based
/// records from an @ref input_stream.
class MLIO_API text_record_reader : public stream_record_reader {
protected:
    explicit
    text_record_reader(intrusive_ptr<input_stream> strm);

private:
    MLIO_HIDDEN
    stdx::optional<record>
    decode_record(memory_slice &chunk, bool ignore_leftover) final;

    /// When implemented in a derived class, tries to decode a record
    /// from the specified chunk.
    ///
    /// @param chunk
    ///     A memory slice that contains one or more records. The chunk
    ///     is guaranteed to have no Unicode BOM bits.
    /// @param ignore_leftover
    ///     A boolean value indicating whether to ignore any leftover
    ///     bits than cannot be interpreted as a record. If false, the
    ///     reader should throw an exception with a descriptive message.
    virtual stdx::optional<record>
    decode_text_record(memory_slice &chunk, bool ignore_leftover) = 0;

    MLIO_HIDDEN
    static bool
    skip_utf8_bom(memory_slice &chunk, bool ignore_leftover) noexcept;
};

/// @}

}  // namespace v1
}  // namespace mlio
