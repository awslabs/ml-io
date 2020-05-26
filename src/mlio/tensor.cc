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

#include "mlio/tensor.h"

#include <cstdlib>
#include <stdexcept>

#include <fmt/format.h>

#include "mlio/tensor_visitor.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

std::string repr(const Tensor &tensor, std::string_view type_name)
{
    return fmt::format("<{0} data_type='{1}' shape=({2}) strides=({3})>",
                       type_name,
                       tensor.data_type(),
                       fmt::join(tensor.shape(), ", "),
                       fmt::join(tensor.strides(), ", "));
}

}  // namespace
}  // namespace detail

Tensor::~Tensor() = default;

Tensor::Tensor(Data_type dt, Size_vector &&shape, Ssize_vector &&strides)
    : data_type_{dt}, shape_{std::move(shape)}, strides_{std::move(strides)}
{
    if (strides_.empty()) {
        strides_ = default_strides(shape_);
    }
    else if (strides_.size() != shape_.size()) {
        throw std::invalid_argument{
            "The number of strides does not match the number of dimensions."};
    }
}

Ssize_vector Tensor::default_strides(const Size_vector &shape)
{
    if (shape.empty()) {
        return {};
    }

    Ssize_vector strides(shape.size(), 1);

    auto d = shape.rbegin();
    for (auto s = strides.rbegin() + 1; s < strides.rend(); ++s, ++d) {
        *s = *(s - 1) * as_ssize(*d);
    }

    return strides;
}

Dense_tensor::Dense_tensor(Size_vector shape,
                           std::unique_ptr<Device_array> &&data,
                           Ssize_vector strides)
    : Tensor{data->data_type(), std::move(shape), std::move(strides)}, data_{std::move(data)}
{
    validate_data_size();
}

std::string Dense_tensor::repr() const
{
    return detail::repr(*this, "Dense_tensor");
}

void Dense_tensor::accept(Tensor_visitor &visitor)
{
    visitor.visit(*this);
}

void Dense_tensor::accept(Tensor_visitor &visitor) const
{
    visitor.visit(*this);
}

void Dense_tensor::validate_data_size() const
{
    if (shape().empty()) {
        return;
    }

    auto stride_pos = strides().begin();

    std::size_t last_idx = 0;
    for (std::size_t dim : shape()) {
        last_idx += (dim - 1) * as_size(std::abs(*stride_pos));

        ++stride_pos;
    }

    std::size_t expected_size = last_idx + 1;
    if (expected_size > data_->size()) {
        throw std::invalid_argument{
            "The size of the data array does not match the specified shape."};
    }
}

Coo_tensor::Coo_tensor(Size_vector shape,
                       std::unique_ptr<Device_array> &&data,
                       std::vector<std::unique_ptr<Device_array>> &&coordinates)
    : Tensor{data->data_type(), std::move(shape), {}}
    , data_{std::move(data)}
    , coordinates_{std::move(coordinates)}
{
    if (this->shape().size() != coordinates_.size()) {
        throw std::invalid_argument{
            "The number of coordinate vectors does not match the number of dimensions."};
    }

    for (auto &indices : coordinates_) {
        if (indices->size() != data_->size()) {
            throw std::invalid_argument{
                "The size of at least one coordinate vector does not match the size of the data array."};
        }
    }
}

std::string Coo_tensor::repr() const
{
    return detail::repr(*this, "Coo_tensor");
}

void Coo_tensor::accept(Tensor_visitor &visitor)
{
    visitor.visit(*this);
}

void Coo_tensor::accept(Tensor_visitor &visitor) const
{
    visitor.visit(*this);
}

Csr_tensor::Csr_tensor(Size_vector shape,
                       std::unique_ptr<Device_array> &&data,
                       std::unique_ptr<Device_array> &&indices,
                       std::unique_ptr<Device_array> &&indptr)
    : Tensor{data->data_type(), std::move(shape), {}}
    , data_{std::move(data)}
    , indices_{std::move(indices)}
    , indptr_{std::move(indptr)}
{
    const Size_vector &shp = this->shape();

    if (shp.size() > 2) {
        throw std::invalid_argument{"A CSR tensor cannot have a rank greater than 2."};
    }

    if (data_->size() != indices_->size()) {
        throw std::invalid_argument{
            "The size of the data array does not match the size of the index array."};
    }

    std::size_t num_rows{};
    if (shp.empty()) {
        num_rows = 0;
    }
    else if (shp.size() == 1) {
        num_rows = 1;
    }
    else {
        num_rows = shp[0];
    }

    if (indptr->size() != num_rows + 1) {
        throw std::invalid_argument{
            "The size of the index pointer array does not match the size of the row dimension."};
    }
}

std::string Csr_tensor::repr() const
{
    return detail::repr(*this, "Csr_tensor");
}

void Csr_tensor::accept(Tensor_visitor &visitor)
{
    visitor.visit(*this);
}

void Csr_tensor::accept(Tensor_visitor &visitor) const
{
    visitor.visit(*this);
}

}  // namespace abi_v1
}  // namespace mlio
