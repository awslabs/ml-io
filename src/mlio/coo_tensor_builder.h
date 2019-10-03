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
#include <vector>

#include <tbb/iterators.h>

#include "mlio/cpu_array.h"
#include "mlio/data_type.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/schema.h"
#include "mlio/span.h"
#include "mlio/tensor.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class coo_tensor_builder {
public:
    explicit
    coo_tensor_builder(feature_desc const &desc, std::size_t batch_size)
        : desc_{&desc}, batch_size_{batch_size}, coords_(desc.shape().size())
    {}

    coo_tensor_builder(coo_tensor_builder const &) = delete;

    coo_tensor_builder(coo_tensor_builder &&) = delete;

    virtual
   ~coo_tensor_builder();

public:
    coo_tensor_builder &
    operator=(coo_tensor_builder const &) = delete;

    coo_tensor_builder &
    operator=(coo_tensor_builder &&) = delete;

public:
    virtual intrusive_ptr<tensor>
    build() = 0;

protected:
    bool
    append_core(stdx::span<std::uint64_t const> keys);

    intrusive_ptr<tensor>
    build_core(std::unique_ptr<device_array> &&data);

private:
    feature_desc const *desc_;
    std::size_t batch_size_;
    std::size_t row_idx_{};
    std::vector<std::vector<std::size_t>> coords_{};
};

template<data_type dt>
class coo_tensor_builder_impl final : public coo_tensor_builder {
public:
    using value_type = data_type_t<dt>;

public:
    using coo_tensor_builder::coo_tensor_builder;

public:
    bool
    append(stdx::span<value_type const> data, stdx::span<std::uint64_t const> keys);

    intrusive_ptr<tensor>
    build() final;

private:
    std::vector<value_type> data_{};
};

template<data_type dt>
bool
coo_tensor_builder_impl<dt>::
append(stdx::span<value_type const> data, stdx::span<std::uint64_t const> keys)
{
    data_.insert(data_.end(), data.begin(), data.end());

    return append_core(keys);
}

template<data_type dt>
intrusive_ptr<tensor>
coo_tensor_builder_impl<dt>::
build()
{
    auto data = wrap_cpu_array<dt>(std::move(data_));

    return build_core(std::move(data));
}

template<data_type dt>
struct make_coo_tensor_builder_op {
    std::unique_ptr<coo_tensor_builder>
    operator()(feature_desc const &desc, std::size_t batch_size)
    {
        return std::make_unique<coo_tensor_builder_impl<dt>>(desc, batch_size);
    }
};

inline std::unique_ptr<coo_tensor_builder>
make_coo_tensor_builder(feature_desc const &desc, std::size_t batch_size)
{
    return dispatch<make_coo_tensor_builder_op>(desc.dtype(), desc, batch_size);
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
