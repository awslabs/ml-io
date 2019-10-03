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

#include "mlio/tensor.h"

#include <cstdlib>
#include <stdexcept>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mlio/tensor_visitor.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

std::string
repr(tensor const &tsr, stdx::string_view type_name)
{
    return fmt::format("<{0} data_type='{1}' shape=({2}) strides=({3})>",
        type_name, tsr.dtype(), fmt::join(tsr.shape(), ", "),
        fmt::join(tsr.strides(), ", "));
}

}  // namespace
}  // namespace detail

tensor::
tensor(data_type dt, size_vector &&shape, ssize_vector &&strides)
    : data_type_{dt}, shape_{std::move(shape)}, strides_{std::move(strides)}
{
    if (strides_.empty()) {
        strides_ = default_strides(shape_);
    } else if (strides_.size() != shape_.size()) {
        throw std::invalid_argument{
            "The number of strides does not match the number of dimensions."};
    }
}

tensor::~tensor() = default;

ssize_vector
tensor::
default_strides(size_vector const &shape)
{
    if (shape.empty()) {
        return {};
    }

    ssize_vector strides(shape.size(), 1);

    auto d = shape.rbegin();
    for (auto s = strides.rbegin() + 1; s < strides.rend(); ++s, ++d) {
        *s = *(s - 1) * as_ssize(*d);
    }

    return strides;
}

dense_tensor::
dense_tensor(size_vector shape, std::unique_ptr<device_array> &&data,
             ssize_vector strides)
    : tensor{data->dtype(), std::move(shape), std::move(strides)}
    , data_{std::move(data)}
{
    validate_data_size();
}

void
dense_tensor::
validate_data_size() const
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

std::string
dense_tensor::
repr() const
{
    return detail::repr(*this, "dense_tensor");
}

void
dense_tensor::
accept(tensor_visitor &vst)
{
    vst.visit(*this);
}

void
dense_tensor::
accept(tensor_visitor &vst) const
{
    vst.visit(*this);
}

coo_tensor::
coo_tensor(size_vector shape, std::unique_ptr<device_array> &&data,
           std::vector<std::unique_ptr<device_array>> &&coords)
    : tensor{data->dtype(), std::move(shape), {}}
    , data_{std::move(data)}
    , coordinates_{std::move(coords)}
{
    if (this->shape().size() != coordinates_.size()) {
        throw std::invalid_argument{
            "The number of coordinate vectors does not match the number of "
            "dimensions."};
    }

    for (auto &indices : coordinates_) {
        if (indices->size() != data_->size()) {
            throw std::invalid_argument{
                "The size of at least one coordinate vector does not match "
                "the size of the data array."};
        }
    }
}

std::string
coo_tensor::
repr() const
{
    return detail::repr(*this, "coo_tensor");
}

void
coo_tensor::
accept(tensor_visitor &vst)
{
    vst.visit(*this);
}

void
coo_tensor::
accept(tensor_visitor &vst) const
{
    vst.visit(*this);
}

csr_tensor::
csr_tensor(size_vector shape,
           std::unique_ptr<device_array> &&data,
           std::unique_ptr<device_array> &&indices,
           std::unique_ptr<device_array> &&indptr)
    : tensor{data->dtype(), std::move(shape), {}}
    , data_{std::move(data)}
    , indices_{std::move(indices)}
    , indptr_{std::move(indptr)}
{
    size_vector const &shp = this->shape();

    if (shp.size() > 2) {
        throw std::invalid_argument{
            "A CSR tensor cannot have a rank greater than 2."};
    }

    if (data_->size() != indices_->size()) {
        throw std::invalid_argument{
            "The size of the data array does not match the size of the index "
            "array."};
    }

    std::size_t num_rows;
    if (shp.empty()) {
        num_rows = 0;
    } else if (shp.size() == 1) {
        num_rows = 1;
    } else {
        num_rows = shp[0];
    }

    if (indptr->size() != num_rows + 1) {
        throw std::invalid_argument{
            "The size of the index pointer array does not match the size of "
            "the row dimension."};
    }
}

std::string
csr_tensor::
repr() const
{
    return detail::repr(*this, "csr_tensor");
}

void
csr_tensor::
accept(tensor_visitor &vst)
{
    vst.visit(*this);
}

void
csr_tensor::
accept(tensor_visitor &vst) const
{
    vst.visit(*this);
}

}  // namespace v1
}  // namespace mlio
