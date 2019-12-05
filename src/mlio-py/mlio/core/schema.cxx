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

#include "module.h"

#include <pybind11/stl_bind.h>

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

feature_desc
make_feature_desc(std::string name,
                  data_type dt,
                  size_vector shape,
                  std::optional<ssize_vector> strides,
                  bool sparse)
{
    feature_desc_builder bld{std::move(name), dt, std::move(shape)};

    if (strides) {
        bld.with_strides(std::move(*strides));
    }

    return bld.with_sparsity(sparse).build();
}

}  // namespace

void
register_schema(py::module &m)
{
    py::class_<feature_desc>(m,
                             "FeatureDesc",
                             "Describes a feature which defines a "
                             "measurable property of a dataset.")
        .def(py::init(&make_feature_desc),
             "name"_a,
             "dtype"_a,
             "shape"_a,
             "strides"_a = std::nullopt,
             "sparse"_a = false)
        .def("__eq__",
             [](feature_desc const &self, feature_desc const &other) {
                 return self == other;
             })
        .def("__hash__",
             [](feature_desc const &self) {
                 return std::hash<feature_desc>{}(self);
             })
        .def("__repr__", &feature_desc::repr)
        .def_property_readonly("name", &feature_desc::name)
        .def_property_readonly("dtype", &feature_desc::dtype)
        .def_property_readonly("shape",
                               [](feature_desc &self) -> py::tuple {
                                   return py::cast(self.shape());
                               })
        .def_property_readonly("strides",
                               [](feature_desc &self) -> py::tuple {
                                   return py::cast(self.strides());
                               })
        .def_property_readonly("sparse", &feature_desc::sparse);

    py::bind_vector<std::vector<feature_desc>>(m, "FeatureDescList");

    py::implicitly_convertible<py::list, std::vector<feature_desc>>();

    py::class_<schema, intrusive_ptr<schema>>(
        m,
        "Schema",
        "Represents a schema that contains the descriptions of all the "
        "features containes in a particular dataset.")
        .def(py::init<std::vector<feature_desc>>(), "descs"_a)
        .def("get_index",
             &schema::get_index,
             "name"_a,
             "Returns the index of the feature descriptor with the specified "
             "name in the descriptor list")
        .def("__eq__",
             [](schema const &self, schema const &other) {
                 return self == other;
             })
        .def("__hash__",
             [](schema const &self) {
                 return std::hash<schema>{}(self);
             })
        .def("__repr__", &schema::repr)
        .def_property_readonly("descriptors", &schema::descriptors);
}

}  // namespace pymlio
