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
#include <utility>
#include <vector>

#include "mlio/config.h"
#include "mlio/instance.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a batch of @ref Instance "data instances".
class MLIO_API Instance_batch {
public:
    /// @param index
    ///     The position of the batch relative to other batches read
    ///     from the dataset.
    /// @param instances
    ///     The list of data instances included in this batch.
    /// @param size
    ///     The size of the batch.
    ///
    /// @remark
    ///     In case this is the last batch of the dataset and the value
    ///     of the @ref last_batch_handling is @c pad, the size can be
    ///     greater than the number of data instances.
    explicit Instance_batch(std::size_t index,
                            std::vector<Instance> &&instances,
                            std::size_t size) noexcept
        : index_{index}, instances_{std::move(instances)}, size_{size}
    {}

    std::size_t index() const noexcept
    {
        return index_;
    }

    const std::vector<Instance> &instances() const noexcept
    {
        return instances_;
    }

    std::size_t size() const noexcept
    {
        return size_;
    }

    std::size_t size_bytes() const;

private:
    std::size_t index_;
    std::vector<Instance> instances_;
    std::size_t size_;
    mutable std::size_t size_bytes_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
