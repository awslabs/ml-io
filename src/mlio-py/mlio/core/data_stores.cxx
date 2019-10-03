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

#include <exception>

#include "py_memory_block.h"

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace mliopy {
namespace detail {
namespace {

class py_data_store : public data_store {
public:
    intrusive_ptr<input_stream>
    open_read() const override;

    std::string
    repr() const override;

public:
    std::string const &
    id() const noexcept override;
};

intrusive_ptr<input_stream>
py_data_store::open_read() const
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(
        intrusive_ptr<input_stream>, data_store, open_read, )
}

std::string
py_data_store::repr() const
{
    py::gil_scoped_acquire acq_gil;

    auto repr_func =
        py::module::import("builtins").attr("object").attr("__repr__");

    return repr_func(this).cast<std::string>();
}

std::string const &
py_data_store::id() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(std::string const &, data_store, id, )
    }
    catch (...) {
        std::terminate();
    }
}

intrusive_ptr<in_memory_store>
make_in_memory_store(py::buffer const &buf, compression cmp)
{
    return make_intrusive<in_memory_store>(
        make_intrusive<py_memory_block>(buf), cmp);
}

std::vector<intrusive_ptr<data_store>>
py_list_files(std::vector<std::string> const &pathnames,
              std::string const &pattern,
              list_files_params::predicate_callback &predicate,
              bool mmap,
              compression cmp)
{
    return list_files({pathnames, &pattern, &predicate, mmap, cmp});
}

}  // namespace
}  // namespace detail

void
register_data_stores(py::module &m)
{
    py::enum_<compression>(
        m, "Compression", "Specifies the compression type of a data store.")
        .value("NONE", compression::none)
        .value("INFER", compression::infer)
        .value("GZIP", compression::gzip)
        .value("BZIP2", compression::bzip2)
        .value("ZIP", compression::zip);

    py::class_<data_store, detail::py_data_store, intrusive_ptr<data_store>>(
        m, "DataStore", "Represents a repository of data.")
        .def(py::init<>())
        .def("open_read",
             &data_store::open_read,
             py::call_guard<py::gil_scoped_release>(),
             "Return an ``input_stream`` for reading from the data store.")
        .def("__eq__",
             [](data_store const &self, data_store const &other) {
                 return self == other;
             })
        .def("__hash__",
             [](data_store const &self) {
                 return std::hash<data_store>{}(self);
             })
        .def("__repr__", &data_store::repr)
        .def_property_readonly(
            "id",
            &data_store::id,
            "Returns a unique identifier for the data store.");

    py::class_<file, data_store, intrusive_ptr<file>>(
        m, "File", "Represents a file as a ``data_store``.")
        .def(py::init<std::string, bool, compression>(),
             "patname"_a,
             "mmap"_a = true,
             "compression"_a = compression::infer,
             R"(
            Parameters
            ----------
            pathname : str
                The path to the file.
            mmap : bool
                A boolean value indicating whether the file should be
                memory-mapped.
            compression : Compression
                The compression type of the file. If set to `infer`, the
                compression will be inferred from the filename.
            )");

    py::class_<in_memory_store, data_store, intrusive_ptr<in_memory_store>>(
        m, "InMemoryStore", "Represents a memory block as a ``data_store``.")
        .def(py::init(&detail::make_in_memory_store),
             "buf"_a,
             "compression"_a = compression::none,
             R"(
            Parameters
            ----------
            buf : buffer
                The Python buffer to wrap as a data store.
            compression : Compression
                The compression type of the data.
            )");

    py::class_<sagemaker_pipe, data_store, intrusive_ptr<sagemaker_pipe>>(
        m,
        "SageMakerPipe",
        "Represents an Amazon SageMaker pipe channel as a ``data_store``.")
        .def(py::init<std::string, std::optional<std::size_t>, compression>(),
             "pathname"_a,
             "fifo_id"_a = std::nullopt,
             "compression"_a = compression::none,
             R"(
            Parameters
            ----------
            pathname : str
                The path to the SageMaker pipe channel.
            fifo_id : int, optional
                The FIFO suffix of the SageMaker pipe channel.
            compression : Compression, optional
                The compression type of the data.
            )");

    m.def("list_files",
          &detail::py_list_files,
          "pathnames"_a,
          "pattern"_a = "",
          "predicate"_a = nullptr,
          "mmap"_a = true,
          "compression"_a = compression::infer,
          R"(
        Recursively list all files residing under the specified pathnames.

        Parameters
        ----------
        pathnames : list of strs
            The list of pathnames to traverse.
        pattern : str, optional
            The pattern to match the filenames against.
        predicate : callable
            The callback function for user-specific filtering.
        mmap : bool
            A boolean value indicating whether the files should be
            memory-mapped.
        compression : Compression
            The compression type of the files. If set to `infer`, the
            compression will be inferred from the filenames.
        )");

    m.def(
        "list_files",
        [](std::string const &pathname, std::string const &pattern) {
            return list_files(pathname, pattern);
        },
        "pathname"_a,
        "pattern"_a = "",
        R"(
        Recursively list all files residing under the specified pathnames.

        Parameters
        ----------
        pathname : str
            The pathname to traverse.
        pattern : str, optional
            The pattern to match the filenames against.
        )");
}

}  // namespace mliopy
