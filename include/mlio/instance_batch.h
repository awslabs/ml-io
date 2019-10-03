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
#include <vector>

#include "mlio/config.h"
#include "mlio/instance.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a batch of @ref instance "data instances".
class MLIO_API instance_batch {
public:
    /// @param index
    ///     The position of the batch relative to other batches read
    ///     from the dataset.
    /// @param lst
    ///     The list of data instances included in this batch.
    /// @param size
    ///     The size of the batch.
    ///
    /// @remark
    ///     In case this is the last batch of the dataset and the value
    ///     of the @ref last_batch_handling is @c pad, the size can be
    ///     greater than the number of data instances.
    explicit
    instance_batch(std::size_t index, std::vector<instance> &&lst, std::size_t size) noexcept
        : index_{index}, instances_{std::move(lst)}, size_{size}
    {}

public:
    std::size_t
    index() const noexcept
    {
        return index_;
    }

    std::vector<instance> const &
    instances() const noexcept
    {
        return instances_;
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

private:
    std::size_t index_;
    std::vector<instance> instances_;
    std::size_t size_;
};

/// @}

}  // namespace v1
}  // namespace mlio
