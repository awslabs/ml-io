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

#include <functional>
#include <iostream>
#include <string>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Represents a repository of data.
class MLIO_API data_store : public intrusive_ref_counter<data_store> {
public:
    data_store() noexcept = default;

    data_store(data_store const &) = delete;

    data_store(data_store &&) = delete;

    virtual
   ~data_store();

public:
    data_store &
    operator=(data_store const &) = delete;

    data_store &
    operator=(data_store &&) = delete;

public:
    /// Returns an @ref input_stream for reading from the data store.
    virtual intrusive_ptr<input_stream>
    open_read() const = 0;

    virtual std::string
    repr() const = 0;

public:
    /// Returns a unique identifier for the data store.
    virtual std::string const &
    id() const noexcept = 0;
};

MLIO_API
inline bool
operator==(data_store const &lhs, data_store const &rhs) noexcept
{
    return lhs.id() == rhs.id();
}

MLIO_API
inline bool
operator!=(data_store const &lhs, data_store const &rhs) noexcept
{
    return lhs.id() != rhs.id();
}

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, data_store const &ds)
{
    return strm << ds.repr();
}

/// @}

}  // namespace v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::data_store> {
    inline size_t
    operator()(mlio::data_store const &ds) const noexcept
    {
        return hash<string>{}(ds.id());
    }
};

}  // namespace std
