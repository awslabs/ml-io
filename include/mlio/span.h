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
#include <type_traits>

#include "mlio/byte.h"
#include "mlio/config.h"
#include "mlio/type_traits.h"

namespace mlio {
inline namespace v1 {
namespace stdx {
namespace detail {

using mlio::detail::is_container;

template<typename Container, typename ElementType, typename = void>
struct is_compatible_container : std::false_type {};

template<typename Container, typename ElementType>
struct is_compatible_container<Container, ElementType, std::enable_if_t<
    std::is_convertible<
        std::remove_pointer_t<decltype(std::declval<Container>().data())>(*)[],
        ElementType(*)[]
    >::value>>
    : std::true_type {};

}  // namespace detail

template<typename T>
class MLIO_API span {
public:
    using element_type = T;
    using value_type = typename std::remove_cv<T>::type;
    using index_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = T const *;
    using reference = T &;
    using const_reference = T const &;
    using iterator = T *;
    using const_iterator = T const *;

public:
    constexpr
    span() noexcept
        : data_{}, size_{}
    {}

    constexpr
    span(span const &other) = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr
    span(pointer data, index_type size) noexcept
        : data_{data}, size_{size}
    {}

    constexpr
    span(pointer first, pointer last) noexcept
        : data_{first}, size_{static_cast<index_type>(last - first)}
    {}

    template<typename Container>
    constexpr
    // NOLINTNEXTLINE(google-explicit-constructor)
    span(Container &cont)
        : data_{cont.data()}, size_{static_cast<index_type>(cont.size())}
    {
        static_assert(detail::is_container<Container>::value,
                      "Container must have data() and size() accessors.");

        static_assert(
            detail::is_compatible_container<Container, element_type>::value,
            "The element type of Container must be convertible to the element "
            "type of the span.");
    }

    template<typename Container>
    constexpr
    // NOLINTNEXTLINE(google-explicit-constructor)
    span(Container const &cont)
        : data_{cont.data()}, size_{static_cast<index_type>(cont.size())}
    {
        static_assert(detail::is_container<Container>::value,
                      "Container must have data() and size() accessors.");

        static_assert(
            detail::is_compatible_container<Container const, element_type>::value,
            "The element type of Container must be convertible to the element "
            "type of the span.");
    }

    template<typename U>
    constexpr
    // NOLINTNEXTLINE(google-explicit-constructor)
    span(span<U> const &other) noexcept
        : data_{other.data()}, size_{other.size()}
    {
        static_assert(std::is_convertible<U(*)[], element_type(*)[]>::value,
            "The element type of U must be convertible to the element type of "
            "the span.");
    }

   ~span() = default;

public:
    constexpr span &
    operator=(span const &) noexcept = default;

public:
    constexpr span<element_type>
    subspan(index_type offset) const
    {
        return {data_ + offset, size_ - offset};
    }

    constexpr span<element_type>
    subspan(index_type offset, index_type count) const
    {
        return {data_ + offset, count};
    }

    constexpr span<element_type>
    first(index_type count) const
    {
        return {data_, count};
    }

    constexpr span<element_type>
    last(index_type count) const
    {
        return {data_ + size_ - count, count};
    }

public:
    constexpr reference
    operator[](index_type index) const
    {
        return data_[index];
    }

public:
    constexpr iterator
    begin() const noexcept
    {
        return data_;
    }

    constexpr iterator
    end() const noexcept
    {
        return data_ + size_;
    }

    constexpr const_iterator
    cbegin() const noexcept
    {
        return data_;
    }

    constexpr const_iterator
    cend() const noexcept
    {
        return data_ + size_;
    }

public:
    constexpr pointer
    data() const noexcept
    {
        return data_;
    }

    constexpr index_type
    size() const noexcept
    {
        return size_;
    }

    constexpr index_type
    size_bytes() const noexcept
    {
        return size_ * sizeof(element_type);
    }

    constexpr bool
    empty() const noexcept
    {
        return size_ == 0;
    }

private:
    pointer data_;
    index_type size_;
};

template<class T>
MLIO_API
inline constexpr span<byte const>
as_bytes(span<T> s) noexcept
{
    return {reinterpret_cast<byte const *>(s.data()), s.size_bytes()};
}

template<class T>
MLIO_API
inline constexpr span<byte>
as_writable_bytes(span<T> s) noexcept
{
    static_assert(!std::is_const<T>::value,
                  "The element type of the specified span must be non-const.");

    return {reinterpret_cast<byte *>(s.data()), s.size_bytes()};
}

}  // namespace stdx

template<typename T, typename U>
MLIO_API
inline constexpr stdx::span<T>
as_span(stdx::span<U> s) noexcept
{
    static_assert(std::is_const<T>::value || !std::is_const<U>::value,
        "T must be const or the element type of the specified span must be "
        "non-const.");

    return {reinterpret_cast<T *>(s.data()), s.size_bytes() / sizeof(T)};
}

template<typename Container>
MLIO_API
constexpr decltype(auto)
make_span(Container &cont)
{
    static_assert(detail::is_container<Container>::value,
                  "Container must have data() and size() accessors.");

    return stdx::span<std::remove_pointer_t<decltype(cont.data())>>{cont};
}

using memory_span = stdx::span<stdx::byte const>;

using mutable_memory_span = stdx::span<stdx::byte>;

}  // namespace v1
}  // namespace mlio
