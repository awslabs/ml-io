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

namespace py = pybind11;

using namespace mlio;

namespace mliopy {

void
register_memory_slice(py::module &m)
{
    py::class_<memory_slice>(m, "MemorySlice", py::buffer_protocol())
        .def_buffer([](memory_slice &self) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            auto data = const_cast<std::byte *>(self.data());

            auto size = static_cast<py::ssize_t>(self.size());

            // TODO(balioglu): Make read-only.
            return py::buffer_info(data, 1, "B", size);
        });
}

}  // namespace mliopy
