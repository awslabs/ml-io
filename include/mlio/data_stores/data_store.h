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

#include <functional>
#include <iostream>
#include <string>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Represents a repository of data.
class MLIO_API Data_store : public Intrusive_ref_counter<Data_store> {
public:
    Data_store() noexcept = default;

    Data_store(const Data_store &) = delete;

    Data_store &operator=(const Data_store &) = delete;

    Data_store(Data_store &&) = delete;

    Data_store &operator=(Data_store &&) = delete;

    virtual ~Data_store();

    /// Returns an @ref Input_stream for reading from the data store.
    virtual Intrusive_ptr<Input_stream> open_read() const = 0;

    virtual std::string repr() const = 0;

    /// Returns a unique identifier for the data store.
    virtual const std::string &id() const = 0;
};

MLIO_API
inline bool operator==(const Data_store &lhs, const Data_store &rhs) noexcept
{
    return lhs.id() == rhs.id();
}

MLIO_API
inline bool operator!=(const Data_store &lhs, const Data_store &rhs) noexcept
{
    return lhs.id() != rhs.id();
}

MLIO_API
inline std::ostream &operator<<(std::ostream &s, const Data_store &store)
{
    return s << store.repr();
}

/// @}

}  // namespace abi_v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::Data_store> {
    inline size_t operator()(const mlio::Data_store &store) const noexcept
    {
        return hash<string>{}(store.id());
    }
};

}  // namespace std
