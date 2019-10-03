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

#include "mlio/example.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <tbb/iterators.h>

#include "mlio/optional.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {

example::
example(intrusive_ptr<schema> shm, std::vector<intrusive_ptr<tensor>> features)
    : schema_{std::move(shm)}, features_{std::move(features)}
{
    if (schema_->descriptors().size() != features_.size()) {
        throw std::invalid_argument{
            "The number of the descriptors does not match the number of "
            "specified features."};
    }
}

intrusive_ptr<tensor>
example::
find_feature(std::string const &name) const noexcept
{
    stdx::optional<std::size_t> idx = schema_->get_index(name);
    if (idx == stdx::nullopt) {
        return {};
    }
    return features_[*idx];
}

std::string
example::
repr() const
{
    auto dsc_beg = schema_->descriptors().begin();
    auto dsc_end = schema_->descriptors().end();

    auto ftr_beg = features_.begin();
    auto ftr_end = features_.end();

    auto pair_beg = tbb::make_zip_iterator(dsc_beg, ftr_beg);
    auto pair_end = tbb::make_zip_iterator(dsc_end, ftr_end);

    std::string s = "{";
    for (auto pos = pair_beg; pos < pair_end; ++pos) {
        s += fmt::format("'{0}': {1}", std::get<0>(*pos).name(), *std::get<1>(*pos));
        if (pos != pair_end - 1) {
            s += ", ";
        }
    }
    s += "}";

    return fmt::format("<example features={{{0}}} padding={1}>", s, padding);
}

}  // namespace v1
}  // namespace mlio
