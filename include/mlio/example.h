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
#include <iostream>
#include <string>
#include <vector>

#include "mlio/config.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/schema.h"
#include "mlio/tensor.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents an Example that holds a @ref Schema and a set of
/// features.
///
/// @remark
///     An Example does not necessarily have a one-to-one mapping to a
///     @ref Instance "data Instance" in a dataset. More than one data
///     Instance can be @e batched into a single Example.
class MLIO_API Example : public Intrusive_ref_counter<Example> {
public:
    /// @param schema
    ///     The Schema that describes the @p features contained in the
    ///     Example.
    /// @param features
    ///     The features of the Example.
    explicit Example(Intrusive_ptr<const Schema> schema,
                     std::vector<Intrusive_ptr<Tensor>> &&features);

    /// Finds the feature that has the specified name.
    ///
    /// @return
    ///     The @ref Tensor Instance if the feature is found in the
    ///     Example; otherwise an @c std::nullptr.
    Intrusive_ptr<Tensor> find_feature(const std::string &name) const noexcept;

    std::string repr() const;

    const Schema &schema() const noexcept
    {
        return *schema_;
    }

    const std::vector<Intrusive_ptr<Tensor>> &features() const noexcept
    {
        return features_;
    }

    /// @remark
    ///     If the padding is greater than zero, it means that the last
    ///     @e padding number of elements in the batch dimension are
    ///     zero-initialized. This is typically the case for the last
    ///     batch read from a dataset if the size of the dataset is not
    ///     evenly divisible by the batch size.
    std::size_t padding{};

private:
    Intrusive_ptr<const Schema> schema_;
    std::vector<Intrusive_ptr<Tensor>> features_;
};

MLIO_API
inline std::ostream &operator<<(std::ostream &s, const Example &example)
{
    return s << example.repr();
}

/// @}

}  // namespace abi_v1
}  // namespace mlio
