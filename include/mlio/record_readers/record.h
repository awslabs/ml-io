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

#include <utility>

#include "mlio/config.h"
#include "mlio/memory/memory_slice.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup records Records
/// @{

enum class record_kind {
    /// The record contains a whole data instance.
    complete,
    /// The record contains the beginning of a data instance.
    begin,
    /// The record contains a mid-section of a data instance.
    middle,
    /// The record contains the end of a data instance.
    end
};

/// Represents an encoded record read from a dataset.
///
/// @remark
///     Note that for some data formats such as RecordIO a data instance
///     can be split into multiple records; meaning a one-to-one mapping
///     between records and data instances is not always true.
class MLIO_API record {
public:
    explicit
    record(memory_slice payload, record_kind knd = record_kind::complete) noexcept
        : payload_{std::move(payload)}, kind_{knd}
    {}

    record(record const &) noexcept = default;

    record(record &&) noexcept = default;

   ~record() = default;

public:
    record &
    operator=(record const &) noexcept = default;

    record &
    operator=(record &&) noexcept = default;

public:
    memory_slice const &
    payload() const & noexcept
    {
        return payload_;
    }

    memory_slice &&
    payload() && noexcept
    {
        return std::move(payload_);
    }

    record_kind
    kind() const noexcept
    {
        return kind_;
    }

private:
    memory_slice payload_;
    record_kind kind_;
};

/// @}

}  // namespace v1
}  // namespace mlio
