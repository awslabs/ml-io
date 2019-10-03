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
#include <utility>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/device.h"
#include "mlio/device_array.h"
#include "mlio/memory/util.h"
#include "mlio/type_traits.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup tensors Tensors
/// @{

namespace detail {

struct cpu_array_access;

}  // namespace detail

/// Represents a memory region allocated in the main memory of the host
/// system.
template<typename Container>
class MLIO_API cpu_array final : public device_array {
    template<typename>
    friend class cpu_array;

    friend struct detail::cpu_array_access;

private:
    explicit
    cpu_array(data_type dt, Container const &cont)
        : data_type_{dt}, container_{cont}
    {}

    explicit
    cpu_array(data_type dt, Container &&cont)
        : data_type_{dt}, container_{std::move(cont)}
    {}

public:
    std::unique_ptr<device_array>
    clone() const final;

public:
    void *
    data() noexcept final
    {
        return container_.data();
    }

    void const *
    data() const noexcept final
    {
        return container_.data();
    }

    std::size_t
    size() const noexcept final
    {
        return container_.size();
    }

    bool
    empty() const noexcept final
    {
        return container_.empty();
    }

    data_type
    dtype() const noexcept final
    {
        return data_type_;
    }

    device
    get_device() const noexcept final
    {
        return device{device_kind::cpu()};
    }

private:
    data_type data_type_;
    Container container_;
};

template<typename Container>
std::unique_ptr<device_array>
cpu_array<Container>::
clone() const
{
    using T = typename Container::value_type;

    std::vector<T> c{container_.begin(), container_.end()};

    auto *ptr = new cpu_array<std::vector<T>>{data_type_, std::move(c)};

    return wrap_unique(ptr);
}

namespace detail {

struct cpu_array_access {
    template<typename Container>
    MLIO_API
    static inline auto
    wrap(data_type dt, Container &&cont)
    {
        auto *ptr = new cpu_array<Container>{dt, std::forward<Container>(cont)};

        return wrap_unique(ptr);
    }
};

}  // namespace detail

/// Copies or moves a container object into a newly constructed
/// @ref cpu_array instance.
///
/// @param cont
///     An object that conforms with the STL sequence container API.
template<data_type dt, typename Container>
MLIO_API
inline std::unique_ptr<device_array>
wrap_cpu_array(Container &&cont)
{
    using T = typename Container::value_type;

    static_assert(detail::is_container<Container>::value,
                  "Container must have data() and size() accessors.");

    static_assert(std::is_same<T, data_type_t<dt>>::value,
                  "The value type of Container must match the specified data "
                  "type.");

    return detail::cpu_array_access::wrap(dt, std::forward<Container>(cont));
}

/// Allocates a new @ref cpu_array with the specified data type and
/// size.
MLIO_API
std::unique_ptr<device_array>
make_cpu_array(data_type dt, std::size_t size);

/// @}

}  // namespace v1
}  // namespace mlio
