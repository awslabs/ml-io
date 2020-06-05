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
#include <functional>
#include <iostream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/tensor.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Describes an Attribute which defines a measurable property of a
/// dataset.
class MLIO_API Attribute {
public:
    explicit Attribute(std::string name,
                       Data_type dt,
                       Size_vector shape,
                       Ssize_vector strides = {},
                       bool sparse = {});

    std::string repr() const;

    const std::string &name() const noexcept
    {
        return name_;
    }

    Data_type data_type() const noexcept
    {
        return data_type_;
    }

    const Size_vector &shape() const noexcept
    {
        return shape_;
    }

    const Ssize_vector &strides() const noexcept
    {
        return strides_;
    }

    bool sparse() const noexcept
    {
        return sparse_;
    }

private:
    explicit Attribute() noexcept = default;

    std::string name_{};
    Data_type data_type_{};
    Size_vector shape_{};
    Ssize_vector strides_{};
    bool sparse_{};
};

MLIO_API
bool operator==(const Attribute &lhs, const Attribute &rhs) noexcept;

MLIO_API
inline bool operator!=(const Attribute &lhs, const Attribute &rhs) noexcept
{
    return !(lhs == rhs);
}

MLIO_API
inline std::ostream &operator<<(std::ostream &s, const Attribute &attr)
{
    return s << attr.repr();
}

/// Represents a Schema that contains the descriptions of all the
/// features contained in a particular dataset.
class MLIO_API Schema : public Intrusive_ref_counter<Schema> {
public:
    explicit Schema(std::vector<Attribute> attrs);

    /// Returns the index of the Attribute with the specified name.
    std::optional<std::size_t> get_index(const std::string &name) const noexcept;

    std::string repr() const;

    const std::vector<Attribute> &attributes() const noexcept
    {
        return attributes_;
    }

private:
    std::vector<Attribute> attributes_;
    std::unordered_map<std::string, std::size_t> name_index_map_;
};

MLIO_API
bool operator==(const Schema &lhs, const Schema &rhs) noexcept;

MLIO_API
inline bool operator!=(const Schema &lhs, const Schema &rhs) noexcept
{
    return !(lhs == rhs);
}

MLIO_API
inline std::ostream &operator<<(std::ostream &s, const Schema &schema)
{
    return s << schema.repr();
}

/// @}

}  // namespace abi_v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::Attribute> {
    size_t operator()(const mlio::Attribute &attr) const noexcept;
};

template<>
struct MLIO_API hash<mlio::Schema> {
    inline size_t operator()(const mlio::Schema &schema) const noexcept
    {
        return mlio::detail::hash_range(schema.attributes());
    }
};

}  // namespace std
