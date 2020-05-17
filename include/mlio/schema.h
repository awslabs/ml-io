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
#include <utility>
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

/// Describes an attribute which defines a measurable property of a
/// dataset.
class MLIO_API attribute {
    friend class attribute_builder;

public:
    explicit attribute(std::string &&name, data_type dt, size_vector &&shape);

private:
    explicit attribute() noexcept = default;

    void init();

public:
    std::string repr() const;

public:
    const std::string &name() const noexcept
    {
        return name_;
    }

    data_type dtype() const noexcept
    {
        return data_type_;
    }

    const size_vector &shape() const noexcept
    {
        return shape_;
    }

    const ssize_vector &strides() const noexcept
    {
        return strides_;
    }

    bool sparse() const noexcept
    {
        return sparse_;
    }

private:
    std::string name_{};
    data_type data_type_{};
    size_vector shape_{};
    ssize_vector strides_{};
    bool sparse_{};
};

MLIO_API
bool operator==(const attribute &lhs, const attribute &rhs) noexcept;

MLIO_API
inline bool operator!=(const attribute &lhs, const attribute &rhs) noexcept
{
    return !(lhs == rhs);
}

MLIO_API
inline std::ostream &operator<<(std::ostream &strm, const attribute &attr)
{
    return strm << attr.repr();
}

/// Builds a @ref attribute instance.
class MLIO_API attribute_builder {
public:
    explicit attribute_builder(std::string name, data_type dt, size_vector shape) noexcept
    {
        attr_.name_ = std::move(name);
        attr_.data_type_ = dt;
        attr_.shape_ = std::move(shape);
    }

public:
    attribute_builder &with_sparsity(bool value) noexcept
    {
        attr_.sparse_ = value;

        return *this;
    }

    attribute_builder &with_strides(ssize_vector strides) noexcept
    {
        attr_.strides_ = std::move(strides);

        return *this;
    }

    attribute &&build()
    {
        attr_.init();

        return std::move(attr_);
    }

    // NOLINTNEXTLINE(google-explicit-constructor)
    operator attribute &&()
    {
        return build();
    }

private:
    attribute attr_{};
};

/// Represents a schema that contains the descriptions of all the
/// features contained in a particular dataset.
class MLIO_API schema : public intrusive_ref_counter<schema> {
public:
    explicit schema(std::vector<attribute> attrs);

public:
    /// Returns the index of the attribute with the specified name.
    std::optional<std::size_t> get_index(const std::string &name) const noexcept;

    std::string repr() const;

public:
    std::vector<attribute> const &attributes() const noexcept
    {
        return attributes_;
    }

private:
    std::vector<attribute> attributes_;
    std::unordered_map<std::string, std::size_t> name_index_map_;
};

MLIO_API
bool operator==(const schema &lhs, const schema &rhs) noexcept;

MLIO_API
inline bool operator!=(const schema &lhs, const schema &rhs) noexcept
{
    return !(lhs == rhs);
}

MLIO_API
inline std::ostream &operator<<(std::ostream &strm, const schema &shm)
{
    return strm << shm.repr();
}

/// @}

}  // namespace abi_v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::attribute> {
    size_t operator()(const mlio::attribute &attr) const noexcept;
};

template<>
struct MLIO_API hash<mlio::schema> {
    inline size_t operator()(const mlio::schema &shm) const noexcept
    {
        return mlio::detail::hash_range(shm.attributes());
    }
};

}  // namespace std
