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
#include <optional>
#include <utility>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/memory/memory_slice.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a data instance read from a dataset.
class MLIO_API instance {
public:
    /// @param ds
    ///     The data store that represents the instance.
    explicit instance(const data_store &ds) noexcept : store_{&ds}
    {}

    /// @param ds
    ///     The data store from which the instance was read.
    /// @param index
    ///     The position of the instance in the data store.
    /// @param bits
    ///     The raw data of the instance.
    explicit instance(const data_store &ds, std::size_t index, memory_slice &&bits) noexcept
        : store_{&ds}, index_{index}, bits_{std::move(bits)}
    {}

private:
    memory_slice load_bits_from_store() const;

    memory_slice read_stream(input_stream &strm) const;

    [[noreturn]] void handle_errors() const;

public:
    const data_store &get_data_store() const noexcept
    {
        return *store_;
    }

    std::size_t index() const noexcept
    {
        return index_;
    }

    const memory_slice &bits() const
    {
        // If we do not have instance data, it means that we should
        // treat the whole data store as a single instance.
        if (bits_ == std::nullopt) {
            bits_ = load_bits_from_store();
        }

        return *bits_;
    }

private:
    const data_store *store_;
    std::size_t index_{};
    mutable std::optional<memory_slice> bits_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
