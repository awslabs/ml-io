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

#include <pybind11/pybind11.h>

#include <cstddef>
#include <utility>
#include <vector>

#include <mlio.h>

namespace mliopy {

class py_device_array {
public:
    explicit
    py_device_array(mlio::intrusive_ptr<mlio::tensor> tsr, mlio::device_array_span s)
        : tensor_{std::move(tsr)}, span_{s}
    {}

    py_device_array(py_device_array const &) = delete;

    py_device_array(py_device_array &&) noexcept = default;

   ~py_device_array();

public:
    py_device_array &
    operator=(py_device_array const &) = delete;

    py_device_array &
    operator=(py_device_array &&) noexcept = default;

public:
    void *
    data() noexcept;

    std::size_t
    size() const noexcept
    {
        return span_.size();
    }

    bool
    empty() const noexcept
    {
        return span_.empty();
    }

    mlio::data_type
    dtype() const noexcept
    {
        return span_.dtype();
    }

    mlio::device
    get_device() const noexcept
    {
        return span_.get_device();
    }

private:
    void *
    make_or_get_string_buffer();

private:
    mlio::intrusive_ptr<mlio::tensor> tensor_;
    mlio::device_array_span span_;
    std::vector<PyObject *> string_buf_{};
};

}  // namespace mliopy
