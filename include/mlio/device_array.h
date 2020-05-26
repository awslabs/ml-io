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

#include <cstddef>
#include <memory>
#include <type_traits>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/device.h"
#include "mlio/span.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup tensors Tensors
/// @{

/// Represents a memory region of a specific data type that is stored on
/// a @ref Device.
class MLIO_API Device_array {
public:
    Device_array() noexcept = default;

    Device_array(const Device_array &) = delete;

    Device_array &operator=(const Device_array &) = delete;

    Device_array(Device_array &&) = delete;

    Device_array &operator=(Device_array &&) = delete;

    virtual ~Device_array();

    virtual std::unique_ptr<Device_array> clone() const = 0;

    /// @remark
    ///     The returned pointer is Device specific and might not be
    ///     be accessible in all contexts.
    virtual void *data() noexcept = 0;

    virtual const void *data() const noexcept = 0;

    virtual std::size_t size() const noexcept = 0;

    [[nodiscard]] virtual bool empty() const noexcept = 0;

    virtual Data_type data_type() const noexcept = 0;

    virtual Device device() const noexcept = 0;
};

/// @remark
///     The specified template parameter must match the data type of the
///     array; otherwise any operation on the returned span will likely
///     cause memory corruption.
template<typename T>
MLIO_API
stdx::span<T> as_span(Device_array &arr) noexcept
{
    return {static_cast<T *>(arr.data()), arr.size()};
}

template<typename T>
MLIO_API
stdx::span<const T> as_span(const Device_array &arr) noexcept
{
    return {static_cast<const T *>(arr.data()), arr.size()};
}

/// Represents a span that wraps a Device array.
template<typename Arr>
class MLIO_API Basic_device_array_span {
    template<typename>
    friend class Basic_device_array_span;

public:
    // NOLINTNEXTLINE(google-explicit-constructor)
    Basic_device_array_span(Arr &arr) noexcept : arr_{&arr}
    {}

    template<typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Basic_device_array_span(const Basic_device_array_span<U> &other) noexcept : arr_{other.arr_}
    {
        static_assert(std::is_same<std::add_const_t<U>, Arr>::value,
                      "A Device array view cannot be assigned to a Device array span.");
    }

    auto data() const noexcept
    {
        return arr_->data();
    }

    std::size_t size() const noexcept
    {
        return arr_->size();
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return arr_->empty();
    }

    Data_type data_type() const noexcept
    {
        return arr_->data_type();
    }

    Device device() const noexcept
    {
        return arr_->device();
    }

    template<typename T>
    auto as() const noexcept
    {
        return as_span<T>(*arr_);
    }

private:
    Arr *arr_;
};

using Device_array_span = Basic_device_array_span<Device_array>;
using Device_array_view = Basic_device_array_span<const Device_array>;

/// @}

}  // namespace abi_v1
}  // namespace mlio
