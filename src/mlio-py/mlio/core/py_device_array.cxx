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

#include "py_device_array.h"

#include <string>

#include "module.h"

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {

py_device_array::~py_device_array()
{
    for (py::handle obj : string_buf_) {
        obj.dec_ref();
    }
}

void *py_device_array::data() noexcept
{
    if (span_.dtype() == data_type::string) {
        return make_or_get_string_buffer();
    }
    return span_.data();
}

void *py_device_array::make_or_get_string_buffer()
{
    if (string_buf_.empty()) {
        string_buf_.reserve(span_.size());

        for (py::str obj : span_.as<std::string>()) {
            string_buf_.push_back(obj.release().ptr());
        }
    }

    return string_buf_.data();
}

namespace {

py::buffer_info to_py_buffer(py_device_array &arr)
{
    auto size = static_cast<py::ssize_t>(arr.size());

    std::size_t item_size{};
    std::string fmt{};

    switch (arr.dtype()) {
    case data_type::size:
        item_size = sizeof(std::size_t);
#if SIZE_MAX == UINT32_MAX
        fmt = "i";
#elif SIZE_MAX == UINT64_MAX
        fmt = "q";
#else
#error "Only 32-bit and 64-bit systems are supported."
#endif
        break;
    case data_type::float16:
        item_size = sizeof(std::uint16_t);
        fmt = "e";
        break;
    case data_type::float32:
        item_size = sizeof(float);
        fmt = "f";
        break;
    case data_type::float64:
        item_size = sizeof(double);
        fmt = "d";
        break;
    case data_type::sint8:
        item_size = sizeof(std::int8_t);
        fmt = "b";
        break;
    case data_type::sint16:
        item_size = sizeof(std::int16_t);
        fmt = "h";
        break;
    case data_type::sint32:
        item_size = sizeof(std::int32_t);
        fmt = "i";
        break;
    case data_type::sint64:
        item_size = sizeof(std::int64_t);
        fmt = "q";
        break;
    case data_type::uint8:
        item_size = sizeof(std::uint8_t);
        fmt = "B";
        break;
    case data_type::uint16:
        item_size = sizeof(std::uint16_t);
        fmt = "H";
        break;
    case data_type::uint32:
        item_size = sizeof(std::uint32_t);
        fmt = "I";
        break;
    case data_type::uint64:
        item_size = sizeof(std::uint64_t);
        fmt = "Q";
        break;
    case data_type::string:
        item_size = sizeof(PyObject *);
        fmt = "O";
        break;
    }

    auto is = static_cast<py::ssize_t>(item_size);

    return py::buffer_info(arr.data(), is, fmt, size);
}

}  // namespace

void register_device_array(py::module &m)
{
    py::class_<device_kind>(m,
                            "DeviceKind",
                            "Represents a device kind that has data processing capabilities such "
                            "as CPU or CUDA.")
        .def("__eq__",
             [](device_kind const &self, device_kind const &other) {
                 return self == other;
             })
        .def("__hash__",
             [](device_kind const &self) {
                 return std::hash<device_kind>{}(self);
             })
        .def("__repr__", &device_kind::repr)
        .def_property_readonly_static(
            "cpu",
            [](py::object &) {
                return device_kind::cpu();
            },
            "Gets an instance for the CPU device kind.")
        .def_property_readonly("name", &device_kind::name, "Gets the name of the device kind.");

    py::class_<device>(
        m, "Device", "Represents a particular data processing unit on the host system.")
        .def(py::init<device_kind, std::size_t>(), "kind"_a, "id"_a)
        .def("__eq__",
             [](device const &self, device const &other) {
                 return self == other;
             })
        .def("__hash__",
             [](device const &self) {
                 return std::hash<device>{}(self);
             })
        .def("__repr__", &device::repr)
        .def_property_readonly("kind", &device::kind, "Gets the kind of the device.")
        .def_property_readonly("id", &device::id, "Gets the id of the device.");

    py::class_<py_device_array>(m,
                                "DeviceArray",
                                py::buffer_protocol(),
                                "Represents a memory region of a specific data type that is stored "
                                "on a ``device``.")
        .def_property_readonly("size", &py_device_array::size, "Gets the size of the array.")
        .def_property_readonly("dtype", &py_device_array::dtype, "Gets the data type of the array.")
        .def_property_readonly(
            "device", &py_device_array::get_device, "Gets the device on which the array is stored.")
        .def_buffer(&to_py_buffer);
}

}  // namespace pymlio
