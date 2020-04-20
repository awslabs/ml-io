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

#include "mlio/schema.h"

#include <stdexcept>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mlio/util/cast.h"

template<typename Char>
struct fmt::formatter<mlio::attribute, Char>
    : fmt::v6::internal::fallback_formatter<mlio::attribute, Char> {};

namespace mlio {
inline namespace v1 {

attribute::attribute(std::string &&name, data_type dt, size_vector &&shape)
    : name_{std::move(name)}, data_type_{dt}, shape_{std::move(shape)}
{
    init();
}

void attribute::init()
{
    if (strides_.empty()) {
        strides_ = tensor::default_strides(shape_);
    }
    else if (strides_.size() != shape_.size()) {
        throw std::invalid_argument{
            "The number of strides does not match the number of dimensions."};
    }
}

std::string attribute::repr() const
{
    return fmt::format(
        "<attribute name='{0}' data_type='{1}' shape=({2}) strides=({3}) sparse='{4}'>",
        name_,
        data_type_,
        fmt::join(shape_, ", "),
        fmt::join(strides_, ", "),
        sparse_);
}

bool operator==(attribute const &lhs, attribute const &rhs) noexcept
{
    return lhs.name() == rhs.name() && lhs.dtype() == rhs.dtype() && lhs.sparse() == rhs.sparse() &&
           lhs.shape() == rhs.shape() && lhs.strides() == rhs.strides();
}

schema::schema(std::vector<attribute> attrs) : attributes_{std::move(attrs)}
{
    std::size_t idx = 0;

    for (auto &attr : attributes_) {
        auto pr = name_index_map_.emplace(attr.name(), idx++);
        if (!pr.second) {
            throw std::invalid_argument{fmt::format(
                "The attribute list contains more than one element with the name '{0}'.",
                attr.name())};
        }
    }
}

std::optional<std::size_t> schema::get_index(std::string const &name) const noexcept
{
    auto pos = name_index_map_.find(name);
    if (pos == name_index_map_.end()) {
        return {};
    }
    return pos->second;
}

std::string schema::repr() const
{
    return fmt::format("<schema attributes={{{0}}}>", fmt::join(attributes_, ", "));
}

bool operator==(schema const &lhs, schema const &rhs) noexcept
{
    return lhs.attributes() == rhs.attributes();
}

}  // namespace v1
}  // namespace mlio

namespace std {  // NOLINT(cert-dcl58-cpp)

size_t hash<mlio::attribute>::operator()(mlio::attribute const &attr) const noexcept
{
    size_t seed = 0;

    mlio::detail::hash_combine(seed, attr.name());
    mlio::detail::hash_combine(seed, attr.dtype());
    mlio::detail::hash_combine(seed, attr.sparse());

    mlio::detail::hash_range(seed, attr.shape());
    mlio::detail::hash_range(seed, attr.strides());

    return seed;
}

}  // namespace std
