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

#include <pybind11/pybind11.h>

#include <memory>

#include <arrow/io/interfaces.h>
#include <mlio/data_stores/data_store.h>
#include <mlio/intrusive_ptr.h>
#include <mlio/record_readers/record.h>
#include <mlio/streams/input_stream.h>
#include <mlio/streams/memory_input_stream.h>

#include "arrow_file.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, mlio::Intrusive_ptr<T>, true);

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {

// The memory layout of Arrow's Cython NativeFile type.
struct Py_arrow_native_file {
    PyObject_HEAD void *vtable;
    std::shared_ptr<arrow::io::InputStream> input_stream;
    std::shared_ptr<arrow::io::RandomAccessFile> random_access;
    std::shared_ptr<arrow::io::OutputStream> output_stream;
    int readable;
    int writable;
    int seekable;
    int own_file;
};

static py::object make_py_arrow_native_file(Intrusive_ptr<Input_stream> &&stream)
{
    auto nf_type = py::module::import("pyarrow").attr("NativeFile");

    auto nf_inst = nf_type();

    auto *obj = reinterpret_cast<Py_arrow_native_file *>(nf_inst.ptr());

    obj->random_access = std::make_shared<Arrow_file>(std::move(stream));
    obj->input_stream = obj->random_access;
    obj->output_stream = nullptr;
    obj->readable = 1;
    obj->writable = 0;
    obj->seekable = 1;
    obj->own_file = 1;

    return nf_inst;
}

static py::object as_arrow_file(const Data_store &st)
{
    auto stream = st.open_read();

    return make_py_arrow_native_file(std::move(stream));
}

static py::object as_arrow_file(const Record &record)
{
    auto stream = make_intrusive<Memory_input_stream>(record.payload());

    return make_py_arrow_native_file(std::move(stream));
}

PYBIND11_MODULE(arrow, m)
{
    m.def("as_arrow_file", py::overload_cast<const Data_store &>(&as_arrow_file), "store"_a);

    m.def("as_arrow_file", py::overload_cast<const Record &>(&as_arrow_file), "record"_a);
}

}  // namespace pymlio
