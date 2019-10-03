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

#include "core/module.h"

#include <cstddef>
#include <memory>
#include <vector>

#include "core/py_buffer.h"
#include "core/py_device_array.h"

namespace py = pybind11;
namespace stdx = mlio::stdx;

using namespace pybind11::literals;

namespace mliopy {
namespace detail {
namespace {

mlio::intrusive_ptr<mlio::dense_tensor>
make_dense_tensor(mlio::size_vector shape, py::buffer &data,
                  stdx::optional<mlio::ssize_vector> strides, bool cpy)
{
    std::unique_ptr<mlio::device_array> arr = make_device_array(data, cpy);

    mlio::ssize_vector strds{};
    if (strides) {
        strds = std::move(*strides);
    }

    return mlio::make_intrusive<mlio::dense_tensor>(
        std::move(shape),
        std::move(arr),
        std::move(strds));
}

mlio::intrusive_ptr<mlio::coo_tensor>
make_coo_tensor(mlio::size_vector shape, py::buffer &data,
                std::vector<py::buffer> &coords, bool cpy)
{
    std::unique_ptr<mlio::device_array> arr = make_device_array(data, cpy);

    std::vector<std::unique_ptr<mlio::device_array>> coordinates{};
    coordinates.reserve(coords.size());

    for (py::buffer &indices : coords) {
        coordinates.emplace_back(make_device_array(indices, cpy));
    }

    return mlio::make_intrusive<mlio::coo_tensor>(
        std::move(shape),
        std::move(arr),
        std::move(coordinates));
}

py::buffer_info
to_py_buffer(mlio::dense_tensor &tsr)
{
    auto buf = py::cast(tsr).attr("data").cast<py::buffer>();

    py::buffer_info info = buf.request(/*writable*/ true);

    info.ndim = static_cast<py::ssize_t>(tsr.shape().size());

    info.shape.clear();
    info.shape.reserve(tsr.shape().size());
    for (auto dim : tsr.shape()) {
        info.shape.push_back(static_cast<py::ssize_t>(dim));
    }

    info.strides.clear();
    info.strides.reserve(tsr.strides().size());
    for (auto stride : tsr.strides()) {
        info.strides.push_back(info.itemsize * stride);
    }

    return info;
}

}  // namespace
}  // namespace detail

void
register_tensors(py::module &m)
{
    py::enum_<mlio::data_type>(m, "DataType")
        .value("SIZE",    mlio::data_type::size)
        .value("FLOAT16", mlio::data_type::float16)
        .value("FLOAT32", mlio::data_type::float32)
        .value("FLOAT64", mlio::data_type::float64)
        .value("SINT8",   mlio::data_type::sint8)
        .value("SINT16",  mlio::data_type::sint16)
        .value("SINT32",  mlio::data_type::sint32)
        .value("SINT64",  mlio::data_type::sint64)
        .value("UINT8",   mlio::data_type::uint8)
        .value("UINT16",  mlio::data_type::uint16)
        .value("UINT32",  mlio::data_type::uint32)
        .value("UINT64",  mlio::data_type::uint64)
        .value("STRING",  mlio::data_type::string);

    py::class_<mlio::tensor, mlio::intrusive_ptr<mlio::tensor>>(m, "Tensor",
        R"(
        Represents a multi-dimensional array.

        This is an abstract class that only defines the data type and shape
        of a tensor. Derived types specify how the tensor data is laid out
        in memory.
        )")
        .def("__repr__", &mlio::tensor::repr)
        .def_property_readonly("dtype", &mlio::tensor::dtype,
            "Gets the data type of the tensor.")
        .def_property_readonly("shape", [](mlio::tensor &self) -> py::tuple
            {
                return py::cast(self.shape());
            },
            "Gets the shape of the tensor.")
        .def_property_readonly("strides", [](mlio::tensor &self) -> py::tuple
            {
                return py::cast(self.strides());
            },
            "Gets the strides of the tensor.");

    py::class_<mlio::dense_tensor,
               mlio::tensor,
               mlio::intrusive_ptr<mlio::dense_tensor>>(m, "DenseTensor", py::buffer_protocol(),
        "Represents a tensor that stores its data in a contiguous memory block.")
        .def(py::init<>(&detail::make_dense_tensor),
            "shape"_a, "data"_a, "strides"_a = stdx::nullopt, "copy"_a = true)
        .def_property_readonly("data", [](mlio::dense_tensor &self)
            {
                return py_device_array{mlio::wrap_intrusive(&self), self.data()};
            },
            "Gets the data of the tensor.")
        .def_buffer(&detail::to_py_buffer);

    py::class_<mlio::coo_tensor,
               mlio::tensor,
               mlio::intrusive_ptr<mlio::coo_tensor>>(m, "CooTensor",
        "Represents a tensor that stores its data in coordinate format.")
        .def(py::init<>(&detail::make_coo_tensor),
            "shape"_a, "data"_a, "coords"_a, "copy"_a = true)
        .def_property_readonly("data", [](mlio::coo_tensor &self)
            {
                return py_device_array{mlio::wrap_intrusive(&self), self.data()};
            },
            "Gets the data of the tensor.")
        .def("indices", [](mlio::coo_tensor &self, std::size_t dim)
            {
                return py_device_array{mlio::wrap_intrusive(&self), self.indices(dim)};
            }, "dim"_a,
            "Gets the indices for the specified dimension.");
}

}  // namespace mliopy
