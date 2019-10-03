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

#pragma once

#include <cstddef>
#include <memory>
#include <type_traits>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/device.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup tensors Tensors
/// @{

/// Represents a memory region of a specific data type that is stored on
/// a @ref device.
class MLIO_API device_array {
public:
    device_array() noexcept = default;

    device_array(device_array const &) = delete;

    device_array(device_array &&) = delete;

    virtual
   ~device_array();

public:
    device_array &
    operator=(device_array const &) = delete;

    device_array &
    operator=(device_array &&) = delete;

public:
    virtual std::unique_ptr<device_array>
    clone() const = 0;

public:
    /// @remark
    ///     The returned pointer is device specific and might not be
    ///     be accessible in all contexts.
    virtual void *
    data() noexcept = 0;

    virtual void const *
    data() const noexcept = 0;

    virtual std::size_t
    size() const noexcept = 0;

    virtual bool
    empty() const noexcept = 0;

    virtual data_type
    dtype() const noexcept = 0;

    virtual device
    get_device() const noexcept = 0;
};

/// @remark
///     The specified template parameter must match the data type of the
///     array; otherwise any operation on the returned span will likely
///     cause memory corruption.
template<typename T>
MLIO_API
stdx::span<T>
as_span(device_array &arr) noexcept
{
    return {static_cast<T *>(arr.data()), arr.size()};
}

template<typename T>
MLIO_API
stdx::span<T const>
as_span(device_array const &arr) noexcept
{
    return {static_cast<T const *>(arr.data()), arr.size()};
}

/// Represents a span that wraps a device array.
template<typename Arr>
class MLIO_API basic_device_array_span {
    template<typename>
    friend class basic_device_array_span;

public:
    // NOLINTNEXTLINE(google-explicit-constructor)
    basic_device_array_span(Arr &arr) noexcept
        : arr_{&arr}
    {}

    template<typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    basic_device_array_span(basic_device_array_span<U> const &other) noexcept
        : arr_{other.arr_}
    {
        static_assert(std::is_same<std::add_const_t<U>, Arr>::value,
            "A device array view cannot be assigned to a device array span.");
    }

public:
    auto
    data() const noexcept
    {
        return arr_->data();
    }

    std::size_t
    size() const noexcept
    {
        return arr_->size();
    }

    bool
    empty() const noexcept
    {
        return arr_->empty();
    }

    data_type
    dtype() const noexcept
    {
        return arr_->dtype();
    }

    device
    get_device() const noexcept
    {
        return arr_->get_device();
    }

public:
    template<typename T>
    auto
    as() const noexcept
    {
        return as_span<T>(*arr_);
    }

private:
    Arr *arr_;
};

using device_array_span = basic_device_array_span<device_array>;
using device_array_view = basic_device_array_span<device_array const>;

/// @}

}  // namespace v1
}  // namespace mlio
