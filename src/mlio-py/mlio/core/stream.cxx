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

#include <exception>

#include "py_memory_block.h"

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

class py_input_stream : public input_stream {
public:
    std::size_t read(mutable_memory_span dest) override;

    memory_slice read(std::size_t size) override;

    void seek(std::size_t position) override;

    void close() noexcept override;

public:
    std::size_t size() const override;

    std::size_t position() const override;

    bool seekable() const noexcept override;

    bool supports_zero_copy() const noexcept override
    {
        return false;
    }

    bool closed() const noexcept override;
};

std::size_t py_input_stream::read(mutable_memory_span dest)
{
    auto bits = as_span<char>(dest);

    auto size = static_cast<py::ssize_t>(bits.size());

    py::gil_scoped_release acq_gil{};

    ::PyObject *buf = ::PyMemoryView_FromMemory(bits.data(), size, PyBUF_WRITE);
    if (buf == nullptr) {
        throw py::error_already_set();
    }

    auto holder = py::reinterpret_steal<py::object>(buf);

    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(std::size_t, input_stream, "read", buf)
}

memory_slice py_input_stream::read(std::size_t)
{
    return {};  // TODO(balioglu): Implement!
}

void py_input_stream::seek(std::size_t position)
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(void, input_stream, "seek", position)
    }
    catch (...) {
        std::terminate();
    }
}

void py_input_stream::close() noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(void, input_stream, "close", )
    }
    catch (...) {
        std::terminate();
    }
}

std::size_t py_input_stream::size() const
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(std::size_t, input_stream, "size", )
}

std::size_t py_input_stream::position() const
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(std::size_t, input_stream, "position", )
}

bool py_input_stream::seekable() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(bool, input_stream, "seekable", )
    }
    catch (...) {
        std::terminate();
    }
}

bool py_input_stream::closed() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(bool, input_stream, "closed", )
    }
    catch (...) {
        std::terminate();
    }
}

std::size_t read_input_stream(input_stream &strm, py::buffer const &buf)
{
    py_mutable_memory_block blk{buf};

    py::gil_scoped_release rel_gil{};

    return strm.read(blk);
}

}  // namespace

void register_streams(py::module &m)
{
    py::class_<input_stream, py_input_stream, intrusive_ptr<input_stream>>(
        m, "InputStream", "Represents an input stream of bytes.")
        .def(py::init<>())
        .def("read",
             &read_input_stream,
             "buf"_a,
             "Fills the specified buffer with data read from the stream.")
        .def("read", py::overload_cast<std::size_t>(&input_stream::read), "size"_a)
        .def("seek",
             &input_stream::seek,
             "position"_a,
             "Seek to the specified position in the stream.")
        .def("close", &input_stream::close)
        .def("__enter__",
             [](input_stream &self) -> input_stream & {
                 return self;
             })
        .def("__exit__",
             [](input_stream &self, py::args const &) {
                 self.close();
             })
        .def_property_readonly("size", &input_stream::size, "Gets the size of the stream.")
        .def_property_readonly(
            "position", &input_stream::position, "Gets the current position in the stream.")
        .def_property_readonly("seekable",
                               &input_stream::seekable,
                               "Gets a boolean value indicating whether the stream is seekable.")
        .def_property_readonly("supports_zero_copy",
                               &input_stream::supports_zero_copy,
                               "Gets a boolean value indicating whether the stream supports "
                               "zero-copy reading.")
        .def_property_readonly("closed",
                               &input_stream::closed,
                               "Gets a boolean value indicating whether the stream is closed.");
}

}  // namespace pymlio
