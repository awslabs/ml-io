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

Intrusive_ptr<Tensor> get_feature(Example &example, const std::string &name)
{
    Intrusive_ptr<Tensor> tensor = example.find_feature(name);
    if (tensor == nullptr) {
        throw py::key_error{"The example does not contain a feature with the specified name."};
    }

    return tensor;
}

Intrusive_ptr<Tensor> get_feature(Example &example, std::size_t index)
{
    if (index >= example.features().size()) {
        throw py::key_error{"The index is out of range."};
    }

    return example.features()[index];
}

}  // namespace

void register_example(py::module &m)
{
    py::class_<Example, Intrusive_ptr<Example>>(
        m, "Example", "Represents an example that holds a ``Schema`` and a set of features.")
        .def(py::init<Intrusive_ptr<const Schema>, std::vector<Intrusive_ptr<Tensor>>>(),
             "schema"_a,
             "features"_a,
             R"(
            Parameters
            ----------
            schema : schema
                The schema that describes the `features` container in the
                example.
            features : list of tensors.
                The features of the example.
            )")
        .def("__len__",
             [](Example &self) {
                 return self.features().size();
             })
        .def("__getitem__", py::overload_cast<Example &, const std::string &>(&get_feature))
        .def("__getitem__", py::overload_cast<Example &, std::size_t>(&get_feature))
        .def("__contains__",
             [](Example &self, const std::string &name) {
                 return self.schema().get_index(name) != std::nullopt;
             })
        .def("__contains__",
             [](Example &self, std::size_t index) {
                 return index < self.features().size();
             })
        .def("__iter__",
             [](Example &self) {
                 return py::make_iterator(self.features().begin(), self.features().end());
             })
        .def("__repr__", &Example::repr)
        .def_property_readonly("schema", &Example::schema, "Gets the schema of the example.")
        .def_readwrite("padding",
                       &Example::padding,
                       R"(
            If the padding is greater than zero, it means that the last
            `padding` number of elements in the batch dimension are
            zero-initialized. This is typically the case for the last
            batch read from a dataset if the size of the dataset is not
            evenly divisible by the batch size.
            )");
}

}  // namespace pymlio
