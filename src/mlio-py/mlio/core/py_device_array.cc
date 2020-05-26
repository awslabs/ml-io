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

Py_device_array::~Py_device_array()
{
    for (py::handle obj : string_buf_) {
        obj.dec_ref();
    }
}

void *Py_device_array::data() noexcept
{
    if (span_.data_type() == Data_type::string) {
        return make_or_get_string_buffer();
    }
    return span_.data();
}

void *Py_device_array::make_or_get_string_buffer()
{
    if (string_buf_.empty()) {
        string_buf_.reserve(span_.size());
        try {
            for (py::str obj : span_.as<std::string>()) {
                string_buf_.push_back(obj.release().ptr());
            }
        }
        catch (std::runtime_error &) {
            PyErr_SetString(
                PyExc_ValueError,
                "The string Tensor contains an invalid UTF-8 sequence. Please make sure that you "
                "specify an explicit text encoding if the text is supposed to be in a different "
                "encoding.");
            return nullptr;
        }
    }

    return string_buf_.data();
}

namespace {

py::buffer_info to_py_buffer(Py_device_array &arr)
{
    auto size = static_cast<py::ssize_t>(arr.size());

    std::size_t item_size{};
    std::string fmt{};

    switch (arr.data_type()) {
    case Data_type::size:
        item_size = sizeof(std::size_t);
#if SIZE_MAX == UINT32_MAX
        fmt = "i";
#elif SIZE_MAX == UINT64_MAX
        fmt = "q";
#else
#error "Only 32-bit and 64-bit systems are supported."
#endif
        break;
    case Data_type::float16:
        item_size = sizeof(std::uint16_t);
        fmt = "e";
        break;
    case Data_type::float32:
        item_size = sizeof(float);
        fmt = "f";
        break;
    case Data_type::float64:
        item_size = sizeof(double);
        fmt = "d";
        break;
    case Data_type::int8:
        item_size = sizeof(std::int8_t);
        fmt = "b";
        break;
    case Data_type::int16:
        item_size = sizeof(std::int16_t);
        fmt = "h";
        break;
    case Data_type::int32:
        item_size = sizeof(std::int32_t);
        fmt = "i";
        break;
    case Data_type::int64:
        item_size = sizeof(std::int64_t);
        fmt = "q";
        break;
    case Data_type::uint8:
        item_size = sizeof(std::uint8_t);
        fmt = "B";
        break;
    case Data_type::uint16:
        item_size = sizeof(std::uint16_t);
        fmt = "H";
        break;
    case Data_type::uint32:
        item_size = sizeof(std::uint32_t);
        fmt = "I";
        break;
    case Data_type::uint64:
        item_size = sizeof(std::uint64_t);
        fmt = "Q";
        break;
    case Data_type::string:
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
    py::class_<Device_kind>(m,
                            "DeviceKind",
                            "Represents a Device kind that has data processing capabilities such "
                            "as CPU or CUDA.")
        .def("__eq__",
             [](const Device_kind &self, const Device_kind &other) {
                 return self == other;
             })
        .def("__hash__",
             [](const Device_kind &self) {
                 return std::hash<Device_kind>{}(self);
             })
        .def("__repr__", &Device_kind::repr)
        .def_property_readonly_static(
            "cpu",
            [](py::object &) {
                return Device_kind::cpu();
            },
            "Gets an Instance for the CPU Device kind.")
        .def_property_readonly("name", &Device_kind::name, "Gets the name of the Device kind.");

    py::class_<Device>(
        m, "Device", "Represents a particular data processing unit on the host system.")
        .def(py::init<Device_kind, std::size_t>(), "kind"_a, "id"_a)
        .def("__eq__",
             [](const Device &self, const Device &other) {
                 return self == other;
             })
        .def("__hash__",
             [](const Device &self) {
                 return std::hash<Device>{}(self);
             })
        .def("__repr__", &Device::repr)
        .def_property_readonly("kind", &Device::kind, "Gets the kind of the Device.")
        .def_property_readonly("id", &Device::id, "Gets the id of the Device.");

    py::class_<Py_device_array>(m,
                                "DeviceArray",
                                py::buffer_protocol(),
                                "Represents a memory region of a specific data type that is stored "
                                "on a ``Device``.")
        .def_property_readonly("size", &Py_device_array::size, "Gets the size of the array.")
        .def_property_readonly(
            "data_type", &Py_device_array::data_type, "Gets the data type of the array.")
        .def_property_readonly(
            "Device", &Py_device_array::device, "Gets the device on which the array is stored.")
        .def_buffer(&to_py_buffer);
}

}  // namespace pymlio
