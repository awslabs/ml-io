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
#include <vector>

#include <tbb/iterators.h>

#include "mlio/cpu_array.h"
#include "mlio/data_type.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/schema.h"
#include "mlio/span.h"
#include "mlio/tensor.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Coo_tensor_builder {
public:
    explicit Coo_tensor_builder(const Attribute &attr, std::size_t batch_size)
        : attr_{&attr}, batch_size_{batch_size}, coordinates_(attr.shape().size())
    {}

    Coo_tensor_builder(const Coo_tensor_builder &) = delete;

    Coo_tensor_builder &operator=(const Coo_tensor_builder &) = delete;

    Coo_tensor_builder(Coo_tensor_builder &&) = delete;

    Coo_tensor_builder &operator=(Coo_tensor_builder &&) = delete;

    virtual ~Coo_tensor_builder();

    virtual Intrusive_ptr<Tensor> build() = 0;

protected:
    bool append_indices(stdx::span<const std::uint64_t> indices);

    Intrusive_ptr<Tensor> build_core(std::unique_ptr<Device_array> &&data);

private:
    const Attribute *attr_;
    std::size_t batch_size_;
    std::size_t row_idx_{};
    std::vector<std::vector<std::size_t>> coordinates_{};
};

template<Data_type dt>
class Coo_tensor_builder_impl final : public Coo_tensor_builder {
public:
    using value_type = data_type_t<dt>;

    using Coo_tensor_builder::Coo_tensor_builder;

    bool append(stdx::span<const value_type> values, stdx::span<const std::uint64_t> indices);

    Intrusive_ptr<Tensor> build() final;

private:
    std::vector<value_type> data_{};
};

template<Data_type dt>
bool Coo_tensor_builder_impl<dt>::append(stdx::span<const value_type> values,
                                         stdx::span<const std::uint64_t> indices)
{
    data_.insert(data_.end(), values.begin(), values.end());

    return append_indices(indices);
}

template<Data_type dt>
Intrusive_ptr<Tensor> Coo_tensor_builder_impl<dt>::build()
{
    auto data = wrap_cpu_array<dt>(std::move(data_));

    return build_core(std::move(data));
}

template<Data_type dt>
struct make_coo_tensor_builder_op {
    std::unique_ptr<Coo_tensor_builder> operator()(const Attribute &attr, std::size_t batch_size)
    {
        return std::make_unique<Coo_tensor_builder_impl<dt>>(attr, batch_size);
    }
};

inline std::unique_ptr<Coo_tensor_builder>
make_coo_tensor_builder(const Attribute &attr, std::size_t batch_size)
{
    return dispatch<make_coo_tensor_builder_op>(attr.data_type(), attr, batch_size);
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
