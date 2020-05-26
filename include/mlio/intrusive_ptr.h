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

#include <functional>
#include <type_traits>
#include <utility>

#include "mlio/config.h"

namespace mlio {
inline namespace abi_v1 {

template<typename T>
class MLIO_API Intrusive_ptr {
    template<typename U>
    friend class Intrusive_ptr;

public:
    using element_type = T;

    constexpr Intrusive_ptr() noexcept = default;

    // NOLINTNEXTLINE(google-explicit-constructor)
    constexpr Intrusive_ptr(std::nullptr_t) noexcept
    {}

    explicit Intrusive_ptr(T *ptr, bool increment_ref = true) noexcept : ptr_{ptr}
    {
        if (increment_ref) {
            increment_ref_count();
        }
    }

    Intrusive_ptr(const Intrusive_ptr &other) noexcept : ptr_{other.ptr_}
    {
        increment_ref_count();
    }

    template<typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Intrusive_ptr(const Intrusive_ptr<U> &other) noexcept : ptr_{static_cast<T *>(other.ptr_)}
    {
        static_assert(std::is_convertible<U *, T *>::value, "U must be convertible to T.");

        increment_ref_count();
    }

    Intrusive_ptr &operator=(std::nullptr_t) noexcept
    {
        decrement_ref_count();

        ptr_ = nullptr;

        return *this;
    }

    Intrusive_ptr &
    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    operator=(const Intrusive_ptr &other) noexcept
    {
        copy_assign(other.ptr_);

        return *this;
    }

    template<typename U>
    Intrusive_ptr &operator=(const Intrusive_ptr<U> &other) noexcept
    {
        static_assert(std::is_convertible<U *, T *>::value, "U must be convertible to T.");

        copy_assign(static_cast<T *>(other.ptr_));

        return *this;
    }

    Intrusive_ptr(Intrusive_ptr &&other) noexcept : ptr_{other.ptr_}
    {
        other.ptr_ = nullptr;
    }

    template<typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Intrusive_ptr(Intrusive_ptr<U> &&other) noexcept : ptr_{static_cast<T *>(other.ptr_)}
    {
        static_assert(std::is_convertible<U *, T *>::value, "U must be convertible to T.");

        other.ptr_ = nullptr;
    }

    Intrusive_ptr &operator=(Intrusive_ptr &&other) noexcept
    {
        move_assign(other.ptr_);

        other.ptr_ = nullptr;

        return *this;
    }

    template<typename U>
    Intrusive_ptr &operator=(Intrusive_ptr<U> &&other) noexcept
    {
        static_assert(std::is_convertible<U *, T *>::value, "U must be convertible to T.");

        move_assign(static_cast<T *>(other.ptr_));

        other.ptr_ = nullptr;

        return *this;
    }

    ~Intrusive_ptr()
    {
        decrement_ref_count();
    }

    T *release() noexcept
    {
        return std::exchange(ptr_, nullptr);
    }

    explicit constexpr operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

    constexpr T *get() const noexcept
    {
        return ptr_;
    }

    constexpr T *operator->() const noexcept
    {
        return ptr_;
    }

    constexpr T &operator*() const noexcept
    {
        return *ptr_;
    }

private:
    void copy_assign(T *other_ptr) noexcept
    {
        if (ptr_ == other_ptr) {
            return;
        }

        decrement_ref_count();

        ptr_ = other_ptr;

        increment_ref_count();
    }

    void move_assign(T *other_ptr) noexcept
    {
        decrement_ref_count();

        ptr_ = other_ptr;
    }

    void increment_ref_count() noexcept
    {
        if (ptr_ != nullptr) {
            intrusive_ptr_inc_ref(ptr_);
        }
    }

    void decrement_ref_count() noexcept
    {
        if (ptr_ != nullptr) {
            intrusive_ptr_dec_ref(ptr_);
        }
    }

private:
    T *ptr_{};
};

template<typename T>
MLIO_API
inline constexpr bool operator==(const Intrusive_ptr<T> &lhs, const Intrusive_ptr<T> &rhs)
{
    return lhs.get() == rhs.get();
}

template<typename T>
MLIO_API
inline constexpr bool operator!=(const Intrusive_ptr<T> &lhs, const Intrusive_ptr<T> &rhs)
{
    return lhs.get() != rhs.get();
}

template<typename T>
MLIO_API
inline constexpr bool operator==(const Intrusive_ptr<T> &lhs, std::nullptr_t)
{
    return lhs.get() == nullptr;
}

template<typename T>
MLIO_API
inline constexpr bool operator!=(const Intrusive_ptr<T> &lhs, std::nullptr_t)
{
    return lhs.get() != nullptr;
}

template<typename T>
MLIO_API
inline constexpr bool operator==(std::nullptr_t, const Intrusive_ptr<T> &rhs)
{
    return rhs.get() == nullptr;
}

template<typename T>
MLIO_API
inline constexpr bool operator!=(std::nullptr_t, const Intrusive_ptr<T> &rhs)
{
    return rhs.get() != nullptr;
}

template<typename T, typename... Args>
MLIO_API
inline Intrusive_ptr<T> make_intrusive(Args &&... args)
{
    return Intrusive_ptr<T>{new T{std::forward<Args>(args)...}, true};
}

template<typename T>
MLIO_API
inline Intrusive_ptr<T> wrap_intrusive(T *t, bool increment_ref = true)
{
    return Intrusive_ptr<T>{t, increment_ref};
}

}  // namespace abi_v1
}  // namespace mlio

namespace std {

template<typename T>
struct MLIO_API hash<mlio::Intrusive_ptr<T>> {
    inline size_t operator()(const mlio::Intrusive_ptr<T> &ptr) const noexcept
    {
        return hash<T *>{}(ptr.get());
    }
};

}  // namespace std
