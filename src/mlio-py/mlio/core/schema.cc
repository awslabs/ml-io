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

Attribute make_attribute(std::string name,
                         Data_type dt,
                         Size_vector shape,
                         std::optional<Ssize_vector> strides,
                         bool sparse)
{
    Attribute_builder bld{std::move(name), dt, std::move(shape)};

    if (strides) {
        bld.with_strides(std::move(*strides));
    }

    return bld.with_sparsity(sparse).build();
}

}  // namespace

void register_schema(py::module &m)
{
    py::class_<Attribute>(m,
                          "Attribute",
                          "Describes an Attribute which defines a measurable "
                          "property of a dataset.")
        .def(py::init(&make_attribute),
             "name"_a,
             "data_type"_a,
             "shape"_a,
             "strides"_a = std::nullopt,
             "sparse"_a = false)
        .def("__eq__",
             [](const Attribute &self, const Attribute &other) {
                 return self == other;
             })
        .def("__hash__",
             [](const Attribute &self) {
                 return std::hash<Attribute>{}(self);
             })
        .def("__repr__", &Attribute::repr)
        .def_property_readonly("name", &Attribute::name)
        .def_property_readonly("data_type", &Attribute::data_type)
        .def_property_readonly("shape",
                               [](Attribute &self) -> py::tuple {
                                   return py::cast(self.shape());
                               })
        .def_property_readonly("strides",
                               [](Attribute &self) -> py::tuple {
                                   return py::cast(self.strides());
                               })
        .def_property_readonly("sparse", &Attribute::sparse);

    py::bind_vector<std::vector<Attribute>>(m, "AttributeList");

    py::implicitly_convertible<py::list, std::vector<Attribute>>();

    py::class_<Schema, Intrusive_ptr<Schema>>(
        m, "Schema", "Represents a Schema that contains the attributes of a dataset.")
        .def(py::init<std::vector<Attribute>>(), "attrs"_a)
        .def("get_index",
             &Schema::get_index,
             "name"_a,
             "Returns the index of the Attribute with the specified name.")
        .def("__eq__",
             [](const Schema &self, const Schema &other) {
                 return self == other;
             })
        .def("__hash__",
             [](const Schema &self) {
                 return std::hash<Schema>{}(self);
             })
        .def("__repr__", &Schema::repr)
        .def_property_readonly("attributes", &Schema::attributes);
}

}  // namespace pymlio
