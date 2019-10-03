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

#include <exception>

#include "core/py_memory_block.h"

namespace py = pybind11;

using namespace pybind11::literals;

namespace mliopy {
namespace detail {
namespace {

class py_input_stream : public mlio::input_stream {
public:
    std::size_t
    read(mlio::mutable_memory_span dest) override;

    mlio::memory_slice
    read(std::size_t size) override;

    void
    seek(std::size_t position) override;

    void
    close() noexcept override;

public:
    std::size_t
    size() const override;

    std::size_t
    position() const override;

    bool
    seekable() const noexcept override;

    bool
    supports_zero_copy() const noexcept override
    {
        return false;
    }

    bool
    closed() const noexcept override;
};

std::size_t
py_input_stream::
read(mlio::mutable_memory_span dest)
{
    auto bits = mlio::as_span<char>(dest);

    auto size = static_cast<py::ssize_t>(bits.size());

    py::gil_scoped_release acq_gil{};

    ::PyObject *buf = ::PyMemoryView_FromMemory(bits.data(), size, PyBUF_WRITE);
    if (buf == nullptr) {
        throw py::error_already_set();
    }

    auto holder = py::reinterpret_steal<py::object>(buf);

    PYBIND11_OVERLOAD_PURE(std::size_t, mlio::input_stream, "read", buf)
}

mlio::memory_slice
py_input_stream::
read(std::size_t)
{
    return {};  //TODO(balioglu): Implement!
}

void
py_input_stream::
seek(std::size_t position)
{
    try {
        PYBIND11_OVERLOAD_PURE(void, mlio::input_stream, "seek", position)
    }
    catch (...) {
        std::terminate();
    }
}

void
py_input_stream::
close() noexcept
{
    try {
        PYBIND11_OVERLOAD_PURE(void, mlio::input_stream, "close",)
    }
    catch (...) {
        std::terminate();
    }
}

std::size_t
py_input_stream::
size() const
{
    PYBIND11_OVERLOAD_PURE(std::size_t, mlio::input_stream, "size",)
}

std::size_t
py_input_stream::
position() const
{
    PYBIND11_OVERLOAD_PURE(std::size_t, mlio::input_stream, "position",)
}

bool
py_input_stream::
seekable() const noexcept
{
    try {
        PYBIND11_OVERLOAD_PURE(bool, mlio::input_stream, "seekable",)
    }
    catch (...) {
        std::terminate();
    }
}

bool
py_input_stream::
closed() const noexcept
{
    try {
        PYBIND11_OVERLOAD_PURE(bool, mlio::input_stream, "closed",)
    }
    catch (...) {
        std::terminate();
    }
}

std::size_t
read_input_stream(mlio::input_stream &strm, py::buffer const &buf)
{
    py_mutable_memory_block blk{buf};

    py::gil_scoped_release rel_gil{};

    return strm.read(blk);
}

}  // namespace
}  // namespace detail

void
register_streams(py::module &m)
{
    py::class_<mlio::input_stream,
               detail::py_input_stream,
               mlio::intrusive_ptr<mlio::input_stream>>(m, "InputStream",
        "Represents an input stream of bytes.")
        .def(py::init<>())
        .def("read", &detail::read_input_stream, "buf"_a,
           "Fills the specified buffer with data read from the stream.")
        .def("read", py::overload_cast<std::size_t>(&mlio::input_stream::read),
           "size"_a)
        .def("seek", &mlio::input_stream::seek, "position"_a,
            "Seek to the specified position in the stream.")
        .def("close", &mlio::input_stream::close)
        .def("__enter__", [](mlio::input_stream &self) -> mlio::input_stream &
            {
                return self;
            })
        .def("__exit__", [](mlio::input_stream &self, py::args const &)
            {
                self.close();
            })
        .def_property_readonly("size", &mlio::input_stream::size,
            "Gets the size of the stream.")
        .def_property_readonly("position", &mlio::input_stream::position,
            "Gets the current position in the stream.")
        .def_property_readonly("seekable", &mlio::input_stream::seekable,
            "Gets a boolean value indicating whether the stream is seekable.")
        .def_property_readonly("supports_zero_copy",
            &mlio::input_stream::supports_zero_copy,
            "Gets a boolean value indicating whether the stream supports "
            "zero-copy reading.")
        .def_property_readonly("closed", &mlio::input_stream::closed,
            "Gets a boolean value indicating whether the stream is closed.");
}

}  // namespace mliopy
