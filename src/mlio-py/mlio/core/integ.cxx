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

#include <dlpack/dlpack.h>

namespace py = pybind11;

using namespace pybind11::literals;

namespace mliopy {
namespace detail {
namespace {

py::capsule
to_dlpack_capsule(mlio::tensor &tsr, std::size_t version)
{
    void const *managed_tensor = mlio::as_dlpack(tsr, version);

    return py::capsule(managed_tensor, "dltensor", [](::PyObject *obj)
    {
        auto *ptr = ::PyCapsule_GetPointer(obj, "dltensor");
        if (ptr != nullptr) {
            auto *m_tsr = static_cast<::DLManagedTensor *>(ptr);

            // PyTorch does not steal the dlpack struct; so we need to
            // make sure that it gets destructed.
            m_tsr->deleter(m_tsr);
        } else {
            ::PyErr_Clear();
        }
    });
}

}  // namespace
}  // namespace detail

void
register_integ(py::module &m)
{
    m.def("as_dlpack", &detail::to_dlpack_capsule, "tensor"_a, "version"_a,
        "Wraps the specified `tensor` as a DLManagedTensor.");
}

}  // namespace mliopy
