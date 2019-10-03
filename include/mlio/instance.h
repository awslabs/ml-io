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

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/memory/memory_slice.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a data instance read from a dataset.
class MLIO_API instance {
public:
    /// @param ds
    ///     The data store from which the instance was read.
    /// @param index
    ///     The position of the instance in the data store.
    /// @param bits
    ///     The raw data of the instance.
    explicit
    instance(data_store const &ds, std::size_t index, memory_slice bits) noexcept
        : data_store_{&ds}, index_{index}, bits_{std::move(bits)}
    {}

public:
    data_store const &
    get_data_store() const noexcept
    {
        return *data_store_;
    }

    std::size_t
    index() const noexcept
    {
        return index_;
    }

    memory_slice const &
    bits() const noexcept
    {
        return bits_;
    }

private:
    data_store const *data_store_;
    std::size_t index_;
    memory_slice bits_;
};

/// @}

}  // namespace v1
}  // namespace mlio
