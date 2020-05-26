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

class Py_data_store : public Data_store {
public:
    Intrusive_ptr<Input_stream> open_read() const override;

    std::string repr() const override;

public:
    const std::string &id() const noexcept override;
};

Intrusive_ptr<Input_stream> Py_data_store::open_read() const
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(Intrusive_ptr<Input_stream>, Data_store, open_read, )
}

std::string Py_data_store::repr() const
{
    py::gil_scoped_acquire acq_gil;

    auto repr_func = py::module::import("builtins").attr("object").attr("__repr__");

    return repr_func(this).cast<std::string>();
}

const std::string &Py_data_store::id() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(const std::string &, Data_store, id, )
    }
    catch (...) {
        std::terminate();
    }
}

Intrusive_ptr<In_memory_store> make_in_memory_store(const py::buffer &buf, Compression compression)
{
    return make_intrusive<In_memory_store>(make_intrusive<Py_memory_block>(buf), compression);
}

std::vector<Intrusive_ptr<Data_store>>
py_list_files(const std::vector<std::string> &paths,
              const std::string &pattern,
              List_files_params::Predicate_callback &predicate,
              bool memory_map,
              Compression compression)
{
    return list_files({paths, &pattern, &predicate, memory_map, compression});
}

std::vector<Intrusive_ptr<Data_store>>
py_list_s3_objects(const S3_client &client,
                   const std::vector<std::string> &uris,
                   const std::string &pattern,
                   List_files_params::Predicate_callback &predicate,
                   Compression compression)
{
    return list_s3_objects({&client, uris, &pattern, &predicate, compression});
}

}  // namespace

void register_data_stores(py::module &m)
{
    py::enum_<Compression>(m, "Compression", "Specifies the Compression type of a data store.")
        .value("NONE", Compression::none)
        .value("INFER", Compression::infer)
        .value("GZIP", Compression::gzip)
        .value("BZIP2", Compression::bzip2)
        .value("ZIP", Compression::zip);

    py::class_<Data_store, Py_data_store, Intrusive_ptr<Data_store>>(
        m, "DataStore", "Represents a repository of data.")
        .def(py::init<>())
        .def("open_read",
             &Data_store::open_read,
             py::call_guard<py::gil_scoped_release>(),
             "Return an ``Input_stream`` for reading from the data store.")
        .def("__eq__",
             [](const Data_store &self, const Data_store &other) {
                 return self == other;
             })
        .def("__hash__",
             [](const Data_store &self) {
                 return std::hash<Data_store>{}(self);
             })
        .def("__repr__", &Data_store::repr)
        .def_property_readonly(
            "id", &Data_store::id, "Returns a unique identifier for the data store.");

    py::class_<File, Data_store, Intrusive_ptr<File>>(
        m, "File", "Represents a File as a ``DataStore``.")
        .def(py::init<std::string, bool, Compression>(),
             "path"_a,
             "memory_map"_a = true,
             "Compression"_a = Compression::infer,
             R"(
            Parameters
            ----------
            path : str
                The path to the File.
            memory_map : bool
                A boolean value indicating whether the File should be
                memory-mapped.
            Compression : Compression
                The Compression type of the File. If set to `INFER`, the
                Compression will be inferred from the filename.
            )");

    py::class_<In_memory_store, Data_store, Intrusive_ptr<In_memory_store>>(
        m, "InMemoryStore", "Represents a memory block as a ``Data_store``.")
        .def(py::init(&make_in_memory_store),
             "buf"_a,
             "Compression"_a = Compression::none,
             R"(
            Parameters
            ----------
            buf : buffer
                The Python buffer to wrap as a data store.
            Compression : Compression
                The Compression type of the data.
            )");

    py::class_<S3_object, Data_store, Intrusive_ptr<S3_object>>(
        m, "S3Object", "Represents an S3 object as a ``DataStore``.")
        .def(py::init<Intrusive_ptr<S3_client>, std::string, std::string, Compression>(),
             "client"_a,
             "uri"_a,
             "version_id"_a = "",
             "Compression"_a = Compression::infer,
             R"(
            Parameters
            ----------
            client : S3Client
                The `S3Client` to use.
            uri : str
                The URI of the S3 object.
            version_id : str
                The version of the S3 object to read.
            Compression : Compression
                The Compression type of the S3 object. If set to `INFER`, the
                Compression will be inferred from the URI.
            )");

    py::class_<Sagemaker_pipe, Data_store, Intrusive_ptr<Sagemaker_pipe>>(
        m, "SageMakerPipe", "Represents an Amazon SageMaker pipe channel as a ``DataStore``.")
        .def(py::init<std::string, std::chrono::seconds, std::optional<std::size_t>, Compression>(),
             "path"_a,
             "timeout"_a = sagemaker_pipe_default_timeout,
             "fifo_id"_a = std::nullopt,
             "Compression"_a = Compression::none,
             R"(
            Parameters
            ----------
            path : str
                The path to the SageMaker pipe channel.
            timeout : datetime.timedelta
                The duration to wait for data to appear in the SageMaker pipe
                channel.
            fifo_id : int, optional
                The FIFO suffix of the SageMaker pipe channel.
            Compression : Compression, optional
                The Compression type of the data.
            )");

    m.def("list_files",
          &py_list_files,
          "paths"_a,
          "pattern"_a = "",
          "predicate"_a = nullptr,
          "memory_map"_a = true,
          "Compression"_a = Compression::infer,
          R"(
        Recursively list all files residing under the specified paths.

        Parameters
        ----------
        paths : list of strs
            The list of paths to traverse.
        pattern : str, optional
            The pattern to match the filenames against.
        predicate : callable
            The callback function for user-specific filtering.
        memory_map : bool
            A boolean value indicating whether the files should be
            memory-mapped.
        Compression : Compression
            The Compression type of the files. If set to `INFER`, the
            Compression will be inferred from the filenames.
        )");

    m.def("list_files",
          py::overload_cast<const std::string &, const std::string &>(&list_files),
          "path"_a,
          "pattern"_a = "",
          R"(
        Recursively list all files residing under the specified path.

        Parameters
        ----------
        path : str
            The path to traverse.
        pattern : str, optional
            The pattern to match the filenames against.
        )");

    m.def("list_s3_objects",
          &py_list_s3_objects,
          "client"_a,
          "uris"_a,
          "pattern"_a = "",
          "predicate"_a = nullptr,
          "Compression"_a = Compression::infer,
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
        Compression : Compression
            The Compression type of the S3 objects. If set to `INFER`, the
            Compression will be inferred from the URIs.
        )");

    m.def("list_s3_objects",
          py::overload_cast<const S3_client &, const std::string &, const std::string &>(
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
