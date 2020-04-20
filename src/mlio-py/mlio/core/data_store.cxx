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

#include <chrono>
#include <exception>

#include "py_memory_block.h"

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

class py_data_store : public data_store {
public:
    intrusive_ptr<input_stream> open_read() const override;

    std::string repr() const override;

public:
    std::string const &id() const noexcept override;
};

intrusive_ptr<input_stream> py_data_store::open_read() const
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(intrusive_ptr<input_stream>, data_store, open_read, )
}

std::string py_data_store::repr() const
{
    py::gil_scoped_acquire acq_gil;

    auto repr_func = py::module::import("builtins").attr("object").attr("__repr__");

    return repr_func(this).cast<std::string>();
}

std::string const &py_data_store::id() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(std::string const &, data_store, id, )
    }
    catch (...) {
        std::terminate();
    }
}

intrusive_ptr<in_memory_store> make_in_memory_store(py::buffer const &buf, compression cmp)
{
    return make_intrusive<in_memory_store>(make_intrusive<py_memory_block>(buf), cmp);
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

std::vector<intrusive_ptr<data_store>>
py_list_s3_objects(s3_client const &client,
                   std::vector<std::string> const &uris,
                   std::string const &pattern,
                   list_files_params::predicate_callback &predicate,
                   compression cmp)
{
    return list_s3_objects({&client, uris, &pattern, &predicate, cmp});
}

}  // namespace

void register_data_stores(py::module &m)
{
    py::enum_<compression>(m, "Compression", "Specifies the compression type of a data store.")
        .value("NONE", compression::none)
        .value("INFER", compression::infer)
        .value("GZIP", compression::gzip)
        .value("BZIP2", compression::bzip2)
        .value("ZIP", compression::zip);

    py::class_<data_store, py_data_store, intrusive_ptr<data_store>>(
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
            "id", &data_store::id, "Returns a unique identifier for the data store.");

    py::class_<file, data_store, intrusive_ptr<file>>(
        m, "File", "Represents a file as a ``DataStore``.")
        .def(py::init<std::string, bool, compression>(),
             "pathname"_a,
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
                The compression type of the file. If set to `INFER`, the
                compression will be inferred from the filename.
            )");

    py::class_<in_memory_store, data_store, intrusive_ptr<in_memory_store>>(
        m, "InMemoryStore", "Represents a memory block as a ``data_store``.")
        .def(py::init(&make_in_memory_store),
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

    py::class_<s3_object, data_store, intrusive_ptr<s3_object>>(
        m, "S3Object", "Represents an S3 object as a ``DataStore``.")
        .def(py::init<intrusive_ptr<s3_client>, std::string, std::string, compression>(),
             "client"_a,
             "uri"_a,
             "version_id"_a = "",
             "compression"_a = compression::infer,
             R"(
            Parameters
            ----------
            client : S3Client
                The `S3Client` to use.
            uri : str
                The URI of the S3 object.
            version_id : str
                The version of the S3 object to read.
            compression : Compression
                The compression type of the S3 object. If set to `INFER`, the
                compression will be inferred from the URI.
            )");

    py::class_<sagemaker_pipe, data_store, intrusive_ptr<sagemaker_pipe>>(
        m, "SageMakerPipe", "Represents an Amazon SageMaker pipe channel as a ``DataStore``.")
        .def(py::init<std::string, std::chrono::seconds, std::optional<std::size_t>, compression>(),
             "pathname"_a,
             "timeout"_a = sagemaker_pipe_default_timeout,
             "fifo_id"_a = std::nullopt,
             "compression"_a = compression::none,
             R"(
            Parameters
            ----------
            pathname : str
                The path to the SageMaker pipe channel.
            timeout : datetime.timedelta
                The duration to wait for data to appear in the SageMaker pipe
                channel.
            fifo_id : int, optional
                The FIFO suffix of the SageMaker pipe channel.
            compression : Compression, optional
                The compression type of the data.
            )");

    m.def("list_files",
          &py_list_files,
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
            The compression type of the files. If set to `INFER`, the
            compression will be inferred from the filenames.
        )");

    m.def("list_files",
          py::overload_cast<std::string const &, std::string const &>(&list_files),
          "pathname"_a,
          "pattern"_a = "",
          R"(
        Recursively list all files residing under the specified pathname.

        Parameters
        ----------
        pathname : str
            The pathname to traverse.
        pattern : str, optional
            The pattern to match the filenames against.
        )");

    m.def("list_s3_objects",
          &py_list_s3_objects,
          "client"_a,
          "uris"_a,
          "pattern"_a = "",
          "predicate"_a = nullptr,
          "compression"_a = compression::infer,
          R"(
        List all S3 objects residing under the specified URIs.

        Parameters
        ----------
        client : S3Client
            The S3 client to use.
        uris : list of strs
            The list of URIs to traverse.
        pattern : str, optional
            The pattern to match the S3 objects against.
        predicate : callable
            The callback function for user-specific filtering.
        compression : Compression
            The compression type of the S3 objects. If set to `INFER`, the
            compression will be inferred from the URIs.
        )");

    m.def("list_s3_objects",
          py::overload_cast<s3_client const &, std::string const &, std::string const &>(
              &list_s3_objects),
          "client"_a,
          "uri"_a,
          "pattern"_a = "",
          R"(
        List all S3 objects residing under the specified URI.

        Parameters
        ----------
        client : S3Client
            The S3 client to use.
        uri : str
            The URI to traverse.
        pattern : str, optional
            The pattern to match the S3 objects against.
        )");
}

}  // namespace pymlio
