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
#include <iostream>
#include <string>
#include <vector>

#include "mlio/config.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/schema.h"
#include "mlio/tensor.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents an example that holds a @ref schema and a set of
/// features.
///
/// @remark
///     An example does not necessarily have a one-to-one mapping to a
///     @ref instance "data instance" in a dataset. More than one data
///     instance can be @e batched into a single example.
class MLIO_API example : public intrusive_ref_counter<example> {
public:
    /// @param shm
    ///     The schema that describes the @p features contained in the
    ///     example.
    /// @param features
    ///     The features of the example.
    explicit
    example(intrusive_ptr<schema> shm, std::vector<intrusive_ptr<tensor>> features);

public:
    /// Finds the feature that has the specified name.
    ///
    /// @return
    ///     The @ref tensor instance if the feature is found in the
    ///     example; otherwise an @c std::nullptr.
    intrusive_ptr<tensor>
    find_feature(std::string const &name) const noexcept;

    std::string
    repr() const;

public:
    schema const &
    get_schema() const noexcept
    {
        return *schema_;
    }

    std::vector<intrusive_ptr<tensor>> const &
    features() const noexcept
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
    intrusive_ptr<schema> schema_;
    std::vector<intrusive_ptr<tensor>> features_;
};

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, example const &exm)
{
    return strm << exm.repr();
}

/// @}

}  // namespace v1
}  // namespace mlio
