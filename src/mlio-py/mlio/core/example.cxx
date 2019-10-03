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

mlio::intrusive_ptr<mlio::tensor>
get_feature(mlio::example &exm, std::string const &name)
{
    mlio::intrusive_ptr<mlio::tensor> tsr = exm.find_feature(name);
    if (tsr == nullptr) {
        throw py::key_error{
            "The example does not contain a feature with the specified name"};
    }

    return tsr;
}

mlio::intrusive_ptr<mlio::tensor>
get_feature(mlio::example &exm, std::size_t index)
{
    if (index >= exm.features().size()) {
        throw py::key_error{"The index is out of range."};
    }

    return exm.features()[index];
}

}  // namespace
}  // namespace detail

void
register_example(py::module &m)
{
    py::class_<mlio::example, mlio::intrusive_ptr<mlio::example>>(m, "Example",
        "Represents an example that holds a ``schema`` and a set of features.")
        .def(py::init<mlio::intrusive_ptr<mlio::schema>,
                      std::vector<mlio::intrusive_ptr<mlio::tensor>>>(),
            "schema"_a, "features"_a,
            R"(
            Parameters
            ----------
            schema : Schema
                The schema that describes the `features` container in the
                example.
            features : list of FeatureDescs
                The features of the example.
            )")
        .def("__len__", [](mlio::example &self)
            {
                return self.features().size();
            })
        .def("__getitem__", py::overload_cast<mlio::example &, std::string const &>(&detail::get_feature))
        .def("__getitem__", py::overload_cast<mlio::example &, std::size_t>(&detail::get_feature))
        .def("__contains__", [](mlio::example &self, std::string const &name)
            {
                return self.get_schema().get_index(name) != stdx::nullopt;
            })
        .def("__contains__", [](mlio::example &self, std::size_t index)
            {
                return index < self.features().size();
            })
        .def("__iter__", [](mlio::example &self)
            {
                return py::make_iterator(self.features().begin(),
                                         self.features().end());
            })
        .def("__repr__", &mlio::example::repr)
        .def_property_readonly("schema", &mlio::example::get_schema,
            "Gets the schema of the example.")
        .def_readwrite("padding", &mlio::example::padding,
            R"(
            If the padding is greater than zero, it means that the last
            `padding` number of elements in the batch dimension are
            zero-initialized. This is typically the case for the last
            batch read from a dataset if the size of the dataset is not
            evenly divisible by the batch size.
            )");
}

}  // namespace mliopy
