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

#include <pybind11/stl_bind.h>

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

attribute make_attribute(std::string name,
                         data_type dt,
                         size_vector shape,
                         std::optional<ssize_vector> strides,
                         bool sparse)
{
    attribute_builder bld{std::move(name), dt, std::move(shape)};

    if (strides) {
        bld.with_strides(std::move(*strides));
    }

    return bld.with_sparsity(sparse).build();
}

}  // namespace

void register_schema(py::module &m)
{
    py::class_<attribute>(m,
                          "Attribute",
                          "Describes an attribute which defines a measurable "
                          "property of a dataset.")
        .def(py::init(&make_attribute),
             "name"_a,
             "dtype"_a,
             "shape"_a,
             "strides"_a = std::nullopt,
             "sparse"_a = false)
        .def("__eq__",
             [](attribute const &self, attribute const &other) {
                 return self == other;
             })
        .def("__hash__",
             [](attribute const &self) {
                 return std::hash<attribute>{}(self);
             })
        .def("__repr__", &attribute::repr)
        .def_property_readonly("name", &attribute::name)
        .def_property_readonly("dtype", &attribute::dtype)
        .def_property_readonly("shape",
                               [](attribute &self) -> py::tuple {
                                   return py::cast(self.shape());
                               })
        .def_property_readonly("strides",
                               [](attribute &self) -> py::tuple {
                                   return py::cast(self.strides());
                               })
        .def_property_readonly("sparse", &attribute::sparse);

    py::bind_vector<std::vector<attribute>>(m, "AttributeList");

    py::implicitly_convertible<py::list, std::vector<attribute>>();

    py::class_<schema, intrusive_ptr<schema>>(
        m, "Schema", "Represents a schema that contains the attributes of a dataset.")
        .def(py::init<std::vector<attribute>>(), "attrs"_a)
        .def("get_index",
             &schema::get_index,
             "name"_a,
             "Returns the index of the attribute with the specified name.")
        .def("__eq__",
             [](schema const &self, schema const &other) {
                 return self == other;
             })
        .def("__hash__",
             [](schema const &self) {
                 return std::hash<schema>{}(self);
             })
        .def("__repr__", &schema::repr)
        .def_property_readonly("attributes", &schema::attributes);
}

}  // namespace pymlio
