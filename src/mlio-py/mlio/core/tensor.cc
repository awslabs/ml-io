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

#include "module.h"

#include <cstddef>
#include <memory>
#include <vector>

#include "py_buffer.h"
#include "py_device_array.h"

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

Intrusive_ptr<Dense_tensor> make_dense_tensor(Size_vector shape,
                                              py::buffer &data,
                                              std::optional<Ssize_vector> strides,
                                              bool cpy)
{
    std::unique_ptr<Device_array> arr = make_device_array(data, cpy);

    Ssize_vector strds{};
    if (strides) {
        strds = std::move(*strides);
    }

    return make_intrusive<Dense_tensor>(std::move(shape), std::move(arr), std::move(strds));
}

Intrusive_ptr<Coo_tensor>
make_coo_tensor(Size_vector shape, py::buffer &data, std::vector<py::buffer> &coords, bool cpy)
{
    std::unique_ptr<Device_array> arr = make_device_array(data, cpy);

    std::vector<std::unique_ptr<Device_array>> coordinates{};
    coordinates.reserve(coords.size());

    for (py::buffer &indices : coords) {
        coordinates.emplace_back(make_device_array(indices, cpy));
    }

    return make_intrusive<Coo_tensor>(std::move(shape), std::move(arr), std::move(coordinates));
}

py::buffer_info to_py_buffer(Dense_tensor &tensor)
{
    auto buf = py::cast(tensor).attr("data").cast<py::buffer>();

    py::buffer_info info = buf.request(/*writable*/ true);

    info.ndim = static_cast<py::ssize_t>(tensor.shape().size());

    info.shape.clear();
    info.shape.reserve(tensor.shape().size());
    for (auto dim : tensor.shape()) {
        info.shape.push_back(static_cast<py::ssize_t>(dim));
    }

    info.strides.clear();
    info.strides.reserve(tensor.strides().size());
    for (auto stride : tensor.strides()) {
        info.strides.push_back(info.itemsize * stride);
    }

    return info;
}

}  // namespace

void register_tensors(py::module &m)
{
    py::enum_<Data_type>(m, "DataType")
        .value("SIZE", Data_type::size)
        .value("FLOAT16", Data_type::float16)
        .value("FLOAT32", Data_type::float32)
        .value("FLOAT64", Data_type::float64)
        .value("INT8", Data_type::int8)
        .value("INT16", Data_type::int16)
        .value("INT32", Data_type::int32)
        .value("INT64", Data_type::int64)
        .value("UINT8", Data_type::uint8)
        .value("UINT16", Data_type::uint16)
        .value("UINT32", Data_type::uint32)
        .value("UINT64", Data_type::uint64)
        .value("STRING", Data_type::string);

    py::class_<Tensor, Intrusive_ptr<Tensor>>(m,
                                              "Tensor",
                                              R"(
        Represents a multi-dimensional array.

        This is an abstract class that only defines the data type and shape
        of a Tensor. Derived types specify how the Tensor data is laid out
        in memory.
        )")
        .def("__repr__", &Tensor::repr)
        .def_property_readonly("data_type", &Tensor::data_type, "Gets the data type of the Tensor.")
        .def_property_readonly(
            "shape",
            [](Tensor &self) -> py::tuple {
                return py::cast(self.shape());
            },
            "Gets the shape of the Tensor.")
        .def_property_readonly(
            "strides",
            [](Tensor &self) -> py::tuple {
                return py::cast(self.strides());
            },
            "Gets the strides of the Tensor.");

    py::class_<Dense_tensor, Tensor, Intrusive_ptr<Dense_tensor>>(
        m,
        "DenseTensor",
        py::buffer_protocol(),
        "Represents a Tensor that stores its data in a contiguous memory "
        "block.")
        .def(py::init<>(&make_dense_tensor),
             "shape"_a,
             "data"_a,
             "strides"_a = std::nullopt,
             "copy"_a = true)
        .def_property_readonly(
            "data",
            [](Dense_tensor &self) {
                return Py_device_array{wrap_intrusive(&self), self.data()};
            },
            "Gets the data of the Tensor.")
        .def_buffer(&to_py_buffer);

    py::class_<Coo_tensor, Tensor, Intrusive_ptr<Coo_tensor>>(
        m, "CooTensor", "Represents a Tensor that stores its data in coordinate format.")
        .def(py::init<>(&make_coo_tensor), "shape"_a, "data"_a, "coords"_a, "copy"_a = true)
        .def_property_readonly(
            "data",
            [](Coo_tensor &self) {
                return Py_device_array{wrap_intrusive(&self), self.data()};
            },
            "Gets the data of the Tensor.")
        .def(
            "indices",
            [](Coo_tensor &self, std::size_t dim) {
                return Py_device_array{wrap_intrusive(&self), self.indices(dim)};
            },
            "dim"_a,
            "Gets the indices for the specified dimension.");
}

}  // namespace pymlio
