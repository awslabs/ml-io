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
#include <utility>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/device.h"
#include "mlio/device_array.h"
#include "mlio/memory/util.h"
#include "mlio/type_traits.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup tensors Tensors
/// @{

namespace detail {

struct Cpu_array_access;

}  // namespace detail

/// Represents a memory region allocated in the main memory of the host
/// system.
template<typename Container>
class MLIO_API Cpu_array final : public Device_array {
    template<typename>
    friend class Cpu_array;

    friend struct detail::Cpu_array_access;

public:
    std::unique_ptr<Device_array> clone() const final;

    void *data() noexcept final
    {
        return container_.data();
    }

    const void *data() const noexcept final
    {
        return container_.data();
    }

    std::size_t size() const noexcept final
    {
        return container_.size();
    }

    [[nodiscard]] bool empty() const noexcept final
    {
        return container_.empty();
    }

    Data_type data_type() const noexcept final
    {
        return data_type_;
    }

    Device device() const noexcept final
    {
        return Device{Device_kind::cpu()};
    }

private:
    explicit Cpu_array(Data_type dt, const Container &container)
        : data_type_{dt}, container_{container}
    {}

    explicit Cpu_array(Data_type dt, Container &&container)
        : data_type_{dt}, container_{std::move(container)}
    {}

    Data_type data_type_;
    Container container_;
};

template<typename Container>
std::unique_ptr<Device_array> Cpu_array<Container>::clone() const
{
    using T = typename Container::value_type;

    std::vector<T> c{container_.begin(), container_.end()};

    auto *ptr = new Cpu_array<std::vector<T>>{data_type_, std::move(c)};

    return wrap_unique(ptr);
}

namespace detail {

struct Cpu_array_access {
    template<typename Container>
    MLIO_API
    static inline auto wrap(Data_type dt, Container &&container)
    {
        auto *ptr = new Cpu_array<Container>{dt, std::forward<Container>(container)};

        return wrap_unique(ptr);
    }
};

}  // namespace detail

/// Copies or moves a container object into a newly constructed
/// @ref Cpu_array Instance.
///
/// @param container
///     An object that conforms with the STL sequence container API.
template<Data_type dt, typename Container>
MLIO_API
inline std::unique_ptr<Device_array> wrap_cpu_array(Container &&container)
{
    using T = typename Container::value_type;

    static_assert(detail::Is_container<Container>::value,
                  "Container must have data() and size() accessors.");

    static_assert(std::is_same<T, data_type_t<dt>>::value,
                  "The value type of Container must match the specified data type.");

    return detail::Cpu_array_access::wrap(dt, std::forward<Container>(container));
}

/// Allocates a new @ref Cpu_array with the specified data type and
/// size.
MLIO_API
std::unique_ptr<Device_array> make_cpu_array(Data_type dt, std::size_t size);

/// @}

}  // namespace abi_v1
}  // namespace mlio
