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

#include <pybind11/pybind11.h>

#include <memory>

#include <arrow/io/interfaces.h>
#include <mlio.h>

#include "integ/arrow/arrow_file.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, mlio::intrusive_ptr<T>, true);

namespace py = pybind11;

using namespace pybind11::literals;

namespace mliopy {

// The memory layout of Arrow's Cython NativeFile type.
struct py_arrow_native_file {
    PyObject_HEAD
    void *vtable;
    std::shared_ptr<arrow::io::InputStream> input_stream;
    std::shared_ptr<arrow::io::RandomAccessFile> random_access;
    std::shared_ptr<arrow::io::OutputStream> output_stream;
    int is_readable;
    int is_writable;
    int is_seekable;
    int own_file;
};

static py::object
make_py_arrow_native_file(mlio::intrusive_ptr<mlio::input_stream> &&strm)
{
    auto nf_type = py::module::import("pyarrow").attr("NativeFile");

    auto nf_inst = nf_type();

    auto *obj = reinterpret_cast<py_arrow_native_file *>(nf_inst.ptr());

    obj->random_access = std::make_shared<arrow_file>(std::move(strm));
    obj->input_stream = obj->random_access;
    obj->output_stream = nullptr;
    obj->is_readable = 1;
    obj->is_writable = 0;
    obj->is_seekable = 1;
    obj->own_file = 1;

    return nf_inst;
}

static py::object
as_arrow_file(mlio::data_store const &st)
{
    auto strm = st.open_read();

    return make_py_arrow_native_file(std::move(strm));
}

static py::object
as_arrow_file(mlio::record const &rec)
{
    auto strm = mlio::make_intrusive<mlio::memory_input_stream>(rec.payload());

    return make_py_arrow_native_file(std::move(strm));
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

// NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
PYBIND11_MODULE(arrow, m) {
    m.def("as_arrow_file",
          py::overload_cast<mlio::data_store const &>(&as_arrow_file),
          "store"_a,
          "BALIOGLU");

    m.def("as_arrow_file",
          py::overload_cast<mlio::record const &>(&as_arrow_file),
          "record"_a,
          "BALIOGLU");
}

#pragma clang diagnostic pop

}  // namespace mliopy
