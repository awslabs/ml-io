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

#include "core/py_buffer.h"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace py = pybind11;
namespace stdx = mlio::stdx;

namespace mliopy {
namespace detail {
namespace {

template<typename T>
class py_buffer_container {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using reference = T &;
    using const_reference = T const &;
    using pointer = T *;
    using const_pointer = T const *;
    using iterator = T *;
    using const_iterator = T const *;

public:
    explicit
    py_buffer_container(T *data, size_type size, py::buffer_info &&info) noexcept
        : data_{data}, size_{size}, info_{std::move(info)}
    {}

public:
    iterator
    begin() noexcept
    {
        return data_;
    }

    const_iterator
    begin() const noexcept
    {
        return data_;
    }

    iterator
    end() noexcept
    {
        return data_ + size_;
    }

    const_iterator
    end() const noexcept
    {
        return data_ + size_;
    }

    const_iterator
    cbegin() const noexcept
    {
        return data_;
    }

    const_iterator
    cend() const noexcept
    {
        return data_ + size_;
    }

public:
    pointer
    data() noexcept
    {
        return data_;
    }

    const_pointer
    data() const noexcept
    {
        return data_;
    }

    size_type
    size() noexcept
    {
        return size_;
    }

    size_type
    size() const noexcept
    {
        return size_;
    }

    bool
    empty() const noexcept
    {
        return size_ == 0;
    }

private:
    T *data_;
    size_type size_;
    py::buffer_info info_;
};

stdx::optional<mlio::data_type>
get_data_type(py::buffer_info const &info)
{
    std::string const &fmt_str = info.format;

    if (fmt_str.empty() || fmt_str.size() > 2) {
        return {};
    }

    char fmt = fmt_str[0];
    if (fmt_str.size() == 2) {
        if (fmt != '@' && fmt != '=') {
            return {};
        }

        fmt = fmt_str[1];
    }

    auto itemsize = static_cast<std::size_t>(info.itemsize);

    switch (fmt) {
    case 'N':
        if (itemsize == sizeof(std::size_t))   {
            return mlio::data_type::size;
        }
        break;

    case 'e':
        if (itemsize == sizeof(std::uint16_t)) {
            return mlio::data_type::float16;
        }
        break;

    case 'f':
        if (itemsize == sizeof(float))         {
            return mlio::data_type::float32;
        }
        break;

    case 'd':
        if (itemsize == sizeof(double))        {
            return mlio::data_type::float64;
        }
        break;

    case 'b':
        if (itemsize == sizeof(std::int8_t))   {
            return mlio::data_type::sint8;
        }
        break;
    case 'h':
        if (itemsize == sizeof(std::int16_t))  {
            return mlio::data_type::sint16;
        }
        break;
    case 'i':
        if (itemsize == sizeof(std::int32_t))  {
            return mlio::data_type::sint32;
        }
        break;
    case 'l':
        if (itemsize == sizeof(std::int32_t))  {
            return mlio::data_type::sint32;
        } else if (itemsize == sizeof(std::int64_t))  {
            return mlio::data_type::sint64;
        }
        break;
    case 'q':
        if (itemsize == sizeof(std::int64_t))  {
            return mlio::data_type::sint64;
        }
        break;
    case 'B':
        if (itemsize == sizeof(std::uint8_t))  {
            return mlio::data_type::uint8;
        }
        break;
    case 'H':
        if (itemsize == sizeof(std::uint16_t)) {
            return mlio::data_type::uint16;
        }
        break;
    case 'I':
        if (itemsize == sizeof(std::uint32_t)) {
            return mlio::data_type::uint32;
        }
        break;
    case 'L':
        if (itemsize == sizeof(std::uint32_t)) {
            return mlio::data_type::uint32;
        } else if (itemsize == sizeof(std::uint64_t)) {
            return mlio::data_type::uint64;
        }
        break;
    case 'Q':
        if (itemsize == sizeof(std::uint64_t)) {
            return mlio::data_type::uint64;
        }
        break;
    case 'O':
        if (itemsize == sizeof(PyObject *))    {
            return mlio::data_type::string;
        }
        break;
    }

    return {};
}

template<mlio::data_type dt>
struct make_cpu_array_op {
    using T = mlio::data_type_t<dt>;

    std::unique_ptr<mlio::device_array>
    operator()(py::buffer_info &&info, bool cpy)
    {
        auto *data = static_cast<T *>(info.ptr);

        auto size = static_cast<std::size_t>(info.size);

        if (cpy) {
            std::vector<T> lst;

            lst.assign(data, data + size);

            return mlio::wrap_cpu_array<dt>(std::move(lst));
        }

        py_buffer_container<T> cont{data, size, std::move(info)};

        return mlio::wrap_cpu_array<dt>(std::move(cont));
    }
};

template<>
struct make_cpu_array_op<mlio::data_type::string> {
    static std::vector<std::string>
    make_string_list(py::buffer_info const &info)
    {
        auto size = static_cast<std::size_t>(info.size);

        std::vector<std::string> lst;
        lst.reserve(size);

        auto **obj_buffer = static_cast<PyObject **>(info.ptr);

        for (std::size_t i = 0; i < size; i++) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            lst.emplace_back(py::cast<std::string>(obj_buffer[i]));
        }

        return lst;
    }

    std::unique_ptr<mlio::device_array>
    operator()(py::buffer_info &&info, bool cpy)
    {
        if (!cpy) {
            throw std::invalid_argument{
                "A Python string buffer cannot be used without copying."};
        }

        std::vector<std::string> lst = make_string_list(info);

        return mlio::wrap_cpu_array<mlio::data_type::string>(std::move(lst));
    }
};

}  // namespace
}  // namespace detail

std::unique_ptr<mlio::device_array>
make_device_array(py::buffer &buf, bool cpy)
{
    bool writable = !cpy;

    py::buffer_info info = buf.request(writable);

    stdx::optional<mlio::data_type> dt = detail::get_data_type(info);
    if (dt == stdx::nullopt) {
        throw std::invalid_argument{
            "The Python buffer contains an unsupported data type."};
    }

    return mlio::dispatch<detail::make_cpu_array_op>(*dt, std::move(info), cpy);
}

}  // namespace mliopy
