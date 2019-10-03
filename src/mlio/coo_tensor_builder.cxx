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

#include "mlio/coo_tensor_builder.h"

#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {

coo_tensor_builder::~coo_tensor_builder() = default;

bool
coo_tensor_builder::append_core(stdx::span<std::uint64_t const> keys)
{
    // The first element of the shape and the strides corresponds to the
    // batch dimension. We do not need it for computing the indices.

    auto dim_beg = desc_->shape().begin() + 1;
    auto dim_end = desc_->shape().end();

    auto stride_beg = desc_->strides().begin() + 1;
    auto stride_end = desc_->strides().end();

    auto indices_beg = coords_.begin() + 1;
    auto indices_end = coords_.end();

    auto zip_beg = tbb::make_zip_iterator(dim_beg, stride_beg, indices_beg);
    auto zip_end = tbb::make_zip_iterator(dim_end, stride_end, indices_end);

    for (auto uint_key : keys) {
        std::size_t key;
        // On a 32-bit system we might not be able to convert the key
        // from 64-bit to 32-bit without truncating.
        if (!try_narrow(uint_key, key)) {
            return false;
        }

        coords_[0].emplace_back(row_idx_);

        for (auto zip_pos = zip_beg; zip_pos < zip_end; ++zip_pos) {
            std::size_t stride = as_size(std::get<1>(*zip_pos));

            std::size_t d = key / stride;
            std::size_t r = key % stride;

            std::size_t dim_idx = d;

            // Make sure that the index is within the dimension.
            if (dim_idx >= std::get<0>(*zip_pos)) {
                return false;
            }

            std::get<2>(*zip_pos).emplace_back(dim_idx);

            // Use the remainder as the new key.
            key = r;
        }
    }

    row_idx_++;

    return true;
}

intrusive_ptr<tensor>
coo_tensor_builder::build_core(std::unique_ptr<device_array> &&data)
{
    // Wrap index lists into device arrays.
    std::vector<std::unique_ptr<device_array>> layout{};
    layout.reserve(coords_.size());

    for (std::vector<std::size_t> &indices : coords_) {
        auto arr = wrap_cpu_array<data_type::size>(std::move(indices));
        layout.emplace_back(std::move(arr));
    }

    size_vector shape = desc_->shape();

    // The passed batch size can be less than the actual batch size if
    // there is padding.
    shape[0] = batch_size_;

    return make_intrusive<coo_tensor>(
        std::move(shape), std::move(data), std::move(layout));
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
