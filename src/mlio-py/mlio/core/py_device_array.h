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

#include <pybind11/pybind11.h>

#include <cstddef>
#include <utility>
#include <vector>

#include <mlio.h>

namespace pymlio {

class Py_device_array {
public:
    explicit Py_device_array(mlio::Intrusive_ptr<mlio::Tensor> tensor, mlio::Device_array_span s)
        : tensor_{std::move(tensor)}, span_{s}
    {}

    Py_device_array(const Py_device_array &) = delete;

    Py_device_array(Py_device_array &&) noexcept = default;

    ~Py_device_array();

public:
    Py_device_array &operator=(const Py_device_array &) = delete;

    Py_device_array &operator=(Py_device_array &&) noexcept = default;

public:
    void *data() noexcept;

    std::size_t size() const noexcept
    {
        return span_.size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return span_.empty();
    }

    mlio::Data_type data_type() const noexcept
    {
        return span_.data_type();
    }

    mlio::Device device() const noexcept
    {
        return span_.device();
    }

private:
    void *make_or_get_string_buffer();

private:
    mlio::Intrusive_ptr<mlio::Tensor> tensor_;
    mlio::Device_array_span span_;
    std::vector<PyObject *> string_buf_{};
};

}  // namespace pymlio
