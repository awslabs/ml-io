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

#include <pybind11/stl_bind.h>

namespace py = pybind11;
namespace stdx = mlio::stdx;

using namespace pybind11::literals;

namespace mliopy {
namespace detail {
namespace {

mlio::feature_desc
make_feature_desc(std::string name, mlio::data_type dt, mlio::size_vector shape,
                  stdx::optional<mlio::ssize_vector> strides, bool sparse)
{
    mlio::feature_desc_builder bld{std::move(name), dt, std::move(shape)};

    if (strides) {
        bld.with_strides(std::move(*strides));
    }

    return bld.with_sparsity(sparse).build();
}

}  // namespace
}  // namespace detail

void
register_schema(py::module &m)
{
    py::class_<mlio::feature_desc>(m, "FeatureDesc",
        "Describes a feature which defines a measurable property of a dataset.")
        .def(py::init(&detail::make_feature_desc),
            "name"_a, "dtype"_a, "shape"_a, "strides"_a = stdx::nullopt,
            "sparse"_a = false)
        .def("__eq__", [](mlio::feature_desc const &self,
                          mlio::feature_desc const &other)
            {
                return self == other;
            })
        .def("__hash__", [](mlio::feature_desc const &self)
            {
                return std::hash<mlio::feature_desc>{}(self);
            })
        .def("__repr__", &mlio::feature_desc::repr)
        .def_property_readonly("name", &mlio::feature_desc::name)
        .def_property_readonly("dtype", &mlio::feature_desc::dtype)
        .def_property_readonly("shape", [](mlio::feature_desc &self) -> py::tuple
            {
                return py::cast(self.shape());
            })
        .def_property_readonly("strides", [](mlio::feature_desc &self) -> py::tuple
            {
                return py::cast(self.strides());
            })
        .def_property_readonly("sparse", &mlio::feature_desc::sparse);

    py::bind_vector<std::vector<mlio::feature_desc>>(m, "FeatureDescList");

    py::implicitly_convertible<py::list, std::vector<mlio::feature_desc>>();

    py::class_<mlio::schema, mlio::intrusive_ptr<mlio::schema>>(m, "Schema",
        "Represents a schema that contains the descriptions of all the "
        "features containes in a particular dataset.")
        .def(py::init<std::vector<mlio::feature_desc>>(), "descs"_a)
        .def("get_index", &mlio::schema::get_index, "name"_a,
            "Returns the index of the feature descriptor with the specified "
            "name in the descriptor list")
        .def("__eq__", [](mlio::schema const &self, mlio::schema const &other)
            {
                return self == other;
            })
        .def("__hash__", [](mlio::schema const &self)
            {
                return std::hash<mlio::schema>{}(self);
            })
        .def("__repr__", &mlio::schema::repr)
        .def_property_readonly("descriptors", &mlio::schema::descriptors);
}

}  // namespace mliopy
