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

#include "mlio/schema.h"

#include <stdexcept>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {

void
feature_desc::
init()
{
    if (strides_.empty()) {
        strides_ = tensor::default_strides(shape_);
    } else if (strides_.size() != shape_.size()) {
        throw std::invalid_argument{
            "The number of strides does not match the number of dimensions."};
    }
}

std::string
feature_desc::
repr() const
{
    std::string strides;
    if (sparse_) {
        strides = "";
    } else {
        strides = fmt::format(" strides=({0})", fmt::join(strides_, ", "));
    }

    return fmt::format("<feature_desc name='{0}' data_type='{1}' shape=({2}){3} sparse='{4}'>",
        name_, data_type_, fmt::join(shape_, ", "), strides, sparse_);
}

bool
operator==(feature_desc const &lhs, feature_desc const &rhs) noexcept
{
    return lhs.name() == rhs.name() && lhs.dtype() == rhs.dtype() &&
           lhs.sparse() == rhs.sparse() && lhs.shape() == rhs.shape() &&
           lhs.strides() == rhs.strides();
}

schema::
schema(std::vector<feature_desc> descs)
    : descriptors_{std::move(descs)}
{
    std::size_t idx = 0;

    for (auto &desc : descriptors_) {
        auto pr = name_index_map_.emplace(desc.name(), idx++);
        if (!pr.second) {
            throw std::invalid_argument{fmt::format(
                "The feature descriptor list contains more than one element "
                "with the name '{0}'.", desc.name())};
        }
    }
}

stdx::optional<std::size_t>
schema::
get_index(std::string const &name) const noexcept
{
    auto pos = name_index_map_.find(name);
    if (pos == name_index_map_.end()) {
        return {};
    }
    return pos->second;
}

std::string
schema::
repr() const
{
    return fmt::format("<schema descriptors={{{0}}}>", fmt::join(descriptors_, ", "));
}

bool
operator==(schema const &lhs, schema const &rhs) noexcept
{
    return lhs.descriptors() == rhs.descriptors();
}

}  // namespace v1
}  // namespace mlio

namespace std { // NOLINT(cert-dcl58-cpp)

size_t
hash<mlio::feature_desc>::
operator()(mlio::feature_desc const &desc) const noexcept
{
    size_t seed = 0;

    mlio::detail::hash_combine(seed, desc.name());
    mlio::detail::hash_combine(seed, desc.dtype());
    mlio::detail::hash_combine(seed, desc.sparse());

    mlio::detail::hash_range(seed, desc.shape());
    mlio::detail::hash_range(seed, desc.strides());

    return seed;
}

}  // namespace std
