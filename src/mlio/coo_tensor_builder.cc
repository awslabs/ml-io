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

#include "mlio/coo_tensor_builder.h"

#include "mlio/util/cast.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Coo_tensor_builder::~Coo_tensor_builder() = default;

bool Coo_tensor_builder::append_indices(stdx::span<const std::uint64_t> indices)
{
    // The first elements of the shape and the strides correspond to the
    // batch dimension. We do not need them to compute the indices.

    auto dim_beg = attr_->shape().begin() + 1;
    auto dim_end = attr_->shape().end();

    auto stride_beg = attr_->strides().begin() + 1;
    auto stride_end = attr_->strides().end();

    auto coordinates_beg = coordinates_.begin() + 1;
    auto coordinates_end = coordinates_.end();

    auto zip_beg = tbb::make_zip_iterator(dim_beg, stride_beg, coordinates_beg);
    auto zip_end = tbb::make_zip_iterator(dim_end, stride_end, coordinates_end);

    for (auto uint_idx : indices) {
        std::size_t idx{};
        // On a 32-bit system we might not be able to convert the index
        // from 64-bit to 32-bit without narrowing.
        if (!try_narrow(uint_idx, idx)) {
            return false;
        }

        coordinates_[0].emplace_back(row_idx_);

        for (auto zip_pos = zip_beg; zip_pos < zip_end; ++zip_pos) {
            std::size_t stride = as_size(std::get<1>(*zip_pos));

            std::size_t dim_idx = idx / stride;

            // Make sure that the index is within the dimension.
            if (dim_idx >= std::get<0>(*zip_pos)) {
                return false;
            }

            // Put the index to the corresponding coordinate vector.
            std::get<2>(*zip_pos).emplace_back(dim_idx);

            // Use the remainder as the new index.
            idx = idx % stride;
        }
    }

    row_idx_++;

    return true;
}

Intrusive_ptr<Tensor> Coo_tensor_builder::build_core(std::unique_ptr<Device_array> &&data)
{
    // Wrap index lists into device arrays.
    std::vector<std::unique_ptr<Device_array>> layout{};
    layout.reserve(coordinates_.size());

    for (std::vector<std::size_t> &indices : coordinates_) {
        auto arr = wrap_cpu_array<Data_type::size>(std::move(indices));
        layout.emplace_back(std::move(arr));
    }

    Size_vector shape = attr_->shape();

    // The provided batch size can be less than the actual batch size if
    // there is padding.
    shape[0] = batch_size_;

    return make_intrusive<Coo_tensor>(std::move(shape), std::move(data), std::move(layout));
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
