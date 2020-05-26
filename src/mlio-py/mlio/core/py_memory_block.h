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

#pragma once

#include <pybind11/pybind11.h>

#include <mlio.h>

namespace pymlio {

class Py_memory_block final : public mlio::Memory_block {
public:
    explicit Py_memory_block(const pybind11::buffer &buf)
    {
        if (::PyObject_GetBuffer(buf.ptr(), &buffer_, PyBUF_SIMPLE) != 0) {
            throw pybind11::error_already_set();
        }
    }

    Py_memory_block(const Py_memory_block &) = delete;

    Py_memory_block(Py_memory_block &&) = delete;

    ~Py_memory_block() final;

public:
    Py_memory_block &operator=(const Py_memory_block &) = delete;

    Py_memory_block &operator=(Py_memory_block &&) = delete;

public:
    const_pointer data() const noexcept final
    {
        return static_cast<const_pointer>(buffer_.buf);
    }

    size_type size() const noexcept final
    {
        return static_cast<size_type>(buffer_.len);
    }

    const Py_buffer &buffer() const noexcept
    {
        return buffer_;
    }

private:
    Py_buffer buffer_{};
};

class py_mutable_memory_block final : public mlio::Mutable_memory_block {
public:
    explicit py_mutable_memory_block(const pybind11::buffer &buf)
    {
        if (::PyObject_GetBuffer(buf.ptr(), &buffer_, PyBUF_WRITABLE) != 0) {
            throw pybind11::error_already_set();
        }
    }

    py_mutable_memory_block(const py_mutable_memory_block &) = delete;

    py_mutable_memory_block(py_mutable_memory_block &&) = delete;

    ~py_mutable_memory_block() final;

public:
    py_mutable_memory_block &operator=(const py_mutable_memory_block &) = delete;

    py_mutable_memory_block &operator=(py_mutable_memory_block &&) = delete;

public:
    void resize(size_type) final
    {
        throw mlio::Not_supported_error{"The Python buffer does not support resizing."};
    }

public:
    pointer data() noexcept final
    {
        return static_cast<pointer>(buffer_.buf);
    }

    const_pointer data() const noexcept final
    {
        return static_cast<const_pointer>(buffer_.buf);
    }

    size_type size() const noexcept final
    {
        return static_cast<size_type>(buffer_.len);
    }

    bool resizable() const noexcept final
    {
        return false;
    }

private:
    Py_buffer buffer_{};
};

}  // namespace pymlio
