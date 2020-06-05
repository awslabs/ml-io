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

#include <utility>

#include "mlio/config.h"
#include "mlio/memory/memory_slice.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup records Records
/// @{

enum class Record_kind {
    /// The Record contains a whole data Instance.
    complete,
    /// The Record contains the beginning of a data Instance.
    begin,
    /// The Record contains a mid-section of a data Instance.
    middle,
    /// The Record contains the end of a data Instance.
    end
};

/// Represents an encoded Record read from a dataset.
///
/// @remark
///     Note that for some data formats such as RecordIO a data Instance
///     can be split into multiple records; meaning a one-to-one mapping
///     between records and data instances is not always true.
class MLIO_API Record {
public:
    explicit Record(Memory_slice payload, Record_kind kind = {}) noexcept
        : payload_{std::move(payload)}, kind_{kind}
    {}

    Record(const Record &) noexcept = default;

    Record &operator=(const Record &) noexcept = default;

    Record(Record &&) noexcept = default;

    Record &operator=(Record &&) noexcept = default;

    ~Record() = default;

    const Memory_slice &payload() const &noexcept
    {
        return payload_;
    }

    Memory_slice &&payload() &&noexcept
    {
        return std::move(payload_);
    }

    Record_kind kind() const noexcept
    {
        return kind_;
    }

private:
    Memory_slice payload_;
    Record_kind kind_;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
