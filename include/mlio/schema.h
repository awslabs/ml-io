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
#include <functional>
#include <iostream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/optional.h"
#include "mlio/tensor.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Describes a feature which defines a measurable property of a
/// dataset.
class MLIO_API feature_desc {
    friend class feature_desc_builder;

private:
    explicit
    feature_desc(std::string &&name, data_type dt, size_vector &&shape) noexcept
        : name_{std::move(name)}, data_type_{dt}, shape_{std::move(shape)}
    {}

    void
    init();

public:
    std::string
    repr() const;

public:
    std::string const &
    name() const noexcept
    {
        return name_;
    }

    data_type
    dtype() const noexcept
    {
        return data_type_;
    }

    size_vector const &
    shape() const noexcept
    {
        return shape_;
    }

    ssize_vector const &
    strides() const noexcept
    {
        return strides_;
    }

    bool
    sparse() const noexcept
    {
        return sparse_;
    }

private:
    std::string name_;
    data_type data_type_;
    size_vector shape_;
    ssize_vector strides_{};
    bool sparse_{};
};

MLIO_API
bool
operator==(feature_desc const &lhs, feature_desc const &rhs) noexcept;

MLIO_API
inline bool
operator!=(feature_desc const &lhs, feature_desc const &rhs) noexcept
{
    return !(lhs == rhs);
}

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, feature_desc const &desc)
{
    return strm << desc.repr();
}

/// Builds a @ref feature_desc instance.
class MLIO_API feature_desc_builder {
public:
    explicit
    feature_desc_builder(std::string name, data_type dt, size_vector shape) noexcept
        : desc_{std::move(name), dt, std::move(shape)}
    {}

public:
    feature_desc_builder &
    with_sparsity(bool value) noexcept
    {
        desc_.sparse_ = value;

        return *this;
    }

    feature_desc_builder &
    with_strides(ssize_vector strides) noexcept
    {
        desc_.strides_ = std::move(strides);

        return *this;
    }

    feature_desc &&
    build()
    {
        desc_.init();

        return std::move(desc_);
    }

private:
    feature_desc desc_;
};

/// Represents a schema that contains the descriptions of all the
/// features contained in a particular dataset.
class MLIO_API schema : public intrusive_ref_counter<schema> {
public:
    explicit
    schema(std::vector<feature_desc> descs);

public:
    /// Returns the index of the feature descriptor with the specified
    /// name in the descriptor list.
    stdx::optional<std::size_t>
    get_index(std::string const &name) const noexcept;

    std::string
    repr() const;

public:
    std::vector<feature_desc> const &
    descriptors() const noexcept
    {
        return descriptors_;
    }

private:
    std::vector<feature_desc> descriptors_;
    std::unordered_map<std::string, std::size_t> name_index_map_;
};

MLIO_API
bool
operator==(schema const &lhs, schema const &rhs) noexcept;

MLIO_API
inline bool
operator!=(schema const &lhs, schema const &rhs) noexcept
{
    return !(lhs == rhs);
}

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, schema const &shm)
{
    return strm << shm.repr();
}

/// @}

}  // namespace v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::feature_desc> {
    size_t
    operator()(mlio::feature_desc const &desc) const noexcept;
};

template<>
struct MLIO_API hash<mlio::schema> {
    inline size_t
    operator()(mlio::schema const &shm) const noexcept
    {
        return mlio::detail::hash_range(shm.descriptors());
    }
};

}  // namespace std
