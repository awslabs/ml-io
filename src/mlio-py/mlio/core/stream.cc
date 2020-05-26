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

class Py_input_stream : public Input_stream {
public:
    std::size_t read(Mutable_memory_span destination) override;

    Memory_slice read(std::size_t size) override;

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

std::size_t Py_input_stream::read(Mutable_memory_span destination)
{
    auto bits = as_span<char>(destination);

    auto size = static_cast<py::ssize_t>(bits.size());

    py::gil_scoped_release acq_gil{};

    ::PyObject *buf = ::PyMemoryView_FromMemory(bits.data(), size, PyBUF_WRITE);
    if (buf == nullptr) {
        throw py::error_already_set();
    }

    auto holder = py::reinterpret_steal<py::object>(buf);

    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(std::size_t, Input_stream, "read", buf)
}

Memory_slice Py_input_stream::read(std::size_t)
{
    return {};  // TODO(balioglu): Implement!
}

void Py_input_stream::seek(std::size_t position)
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(void, Input_stream, "seek", position)
    }
    catch (...) {
        std::terminate();
    }
}

void Py_input_stream::close() noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(void, Input_stream, "close", )
    }
    catch (...) {
        std::terminate();
    }
}

std::size_t Py_input_stream::size() const
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(std::size_t, Input_stream, "size", )
}

std::size_t Py_input_stream::position() const
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(std::size_t, Input_stream, "position", )
}

bool Py_input_stream::seekable() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(bool, Input_stream, "seekable", )
    }
    catch (...) {
        std::terminate();
    }
}

bool Py_input_stream::closed() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(bool, Input_stream, "closed", )
    }
    catch (...) {
        std::terminate();
    }
}

std::size_t read_input_stream(Input_stream &stream, const py::buffer &buf)
{
    py_mutable_memory_block block{buf};

    py::gil_scoped_release rel_gil{};

    return stream.read(block);
}

}  // namespace

void register_streams(py::module &m)
{
    py::class_<Input_stream, Py_input_stream, Intrusive_ptr<Input_stream>>(
        m, "InputStream", "Represents an input stream of bytes.")
        .def(py::init<>())
        .def("read",
             &read_input_stream,
             "buf"_a,
             "Fills the specified buffer with data read from the stream.")
        .def("read", py::overload_cast<std::size_t>(&Input_stream::read), "size"_a)
        .def("seek",
             &Input_stream::seek,
             "position"_a,
             "Seek to the specified position in the stream.")
        .def("close", &Input_stream::close)
        .def("__enter__",
             [](Input_stream &self) -> Input_stream & {
                 return self;
             })
        .def("__exit__",
             [](Input_stream &self, const py::args &) {
                 self.close();
             })
        .def_property_readonly("size", &Input_stream::size, "Gets the size of the stream.")
        .def_property_readonly(
            "position", &Input_stream::position, "Gets the current position in the stream.")
        .def_property_readonly("seekable",
                               &Input_stream::seekable,
                               "Gets a boolean value indicating whether the stream is seekable.")
        .def_property_readonly("supports_zero_copy",
                               &Input_stream::supports_zero_copy,
                               "Gets a boolean value indicating whether the stream supports "
                               "zero-copy reading.")
        .def_property_readonly("closed",
                               &Input_stream::closed,
                               "Gets a boolean value indicating whether the stream is closed.");
}

}  // namespace pymlio
