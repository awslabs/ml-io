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

#include <functional>
#include <type_traits>
#include <utility>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {

template<typename T>
class MLIO_API intrusive_ptr {
    template<typename U>
    friend class intrusive_ptr;

public:
    using element_type = T;

public:
    constexpr
    intrusive_ptr() noexcept = default;

    constexpr
    // NOLINTNEXTLINE(google-explicit-constructor)
    intrusive_ptr(std::nullptr_t) noexcept
    {}

    explicit
    intrusive_ptr(T *ptr, bool inc_ref = true) noexcept
        : ptr_{ptr}
    {
        if (inc_ref) {
            inc_ref_count();
        }
    }

    intrusive_ptr(intrusive_ptr const &other) noexcept
        : ptr_{other.ptr_}
    {
        inc_ref_count();
    }

    template<typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    intrusive_ptr(intrusive_ptr<U> const &other) noexcept
        : ptr_{static_cast<T *>(other.ptr_)}
    {
        static_assert(std::is_convertible<U *, T *>::value,
                      "U must be convertible to T.");

        inc_ref_count();
    }

    intrusive_ptr(intrusive_ptr &&other) noexcept
        : ptr_{other.ptr_}
    {
        other.ptr_ = nullptr;
    }

    template<typename U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    intrusive_ptr(intrusive_ptr<U> &&other) noexcept
        : ptr_{static_cast<T *>(other.ptr_)}
    {
        static_assert(std::is_convertible<U *, T *>::value,
                      "U must be convertible to T.");

        other.ptr_ = nullptr;
    }

    ~intrusive_ptr()
    {
        dec_ref_count();
    }

public:
    intrusive_ptr &
    operator=(std::nullptr_t) noexcept
    {
        dec_ref_count();

        ptr_ = nullptr;

        return *this;
    }

    intrusive_ptr &
    // NOLINTNEXTLINE(bugprone-unhandled-self-assignment, cert-oop54-cpp)
    operator=(intrusive_ptr const &other) noexcept
    {
        copy_assign(other.ptr_);

        return *this;
    }

    template<typename U>
    intrusive_ptr &
    operator=(intrusive_ptr<U> const &other) noexcept
    {
        static_assert(std::is_convertible<U *, T *>::value,
                      "U must be convertible to T.");

        copy_assign(static_cast<T *>(other.ptr_));

        return *this;
    }

    intrusive_ptr &
    operator=(intrusive_ptr &&other) noexcept
    {
        move_assign(other.ptr_);

        other.ptr_ = nullptr;

        return *this;
    }

    template<typename U>
    intrusive_ptr &
    operator=(intrusive_ptr<U> &&other) noexcept
    {
        static_assert(std::is_convertible<U *, T *>::value,
                      "U must be convertible to T.");

        move_assign(static_cast<T *>(other.ptr_));

        other.ptr_ = nullptr;

        return *this;
    }

private:
    void
    copy_assign(T *other_ptr) noexcept
    {
        if (ptr_ == other_ptr) {
            return;
        }

        dec_ref_count();

        ptr_ = other_ptr;

        inc_ref_count();
    }

    void
    move_assign(T *other_ptr) noexcept
    {
        dec_ref_count();

        ptr_ = other_ptr;
    }

    void
    inc_ref_count() noexcept
    {
        if (ptr_ != nullptr) {
            intrusive_ptr_inc_ref(ptr_);
        }
    }

    void
    dec_ref_count() noexcept
    {
        if (ptr_ != nullptr) {
            intrusive_ptr_dec_ref(ptr_);
        }
    }

public:
    T *
    release() noexcept
    {
        return std::exchange(ptr_, nullptr);
    }

public:
    explicit constexpr
    operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

    constexpr T *
    get() const noexcept
    {
        return ptr_;
    }

    constexpr T *
    operator->() const noexcept
    {
        return ptr_;
    }

    constexpr T &
    operator*() const noexcept
    {
        return *ptr_;
    }

private:
    T *ptr_{};
};

template<typename T>
MLIO_API
inline constexpr bool
operator==(intrusive_ptr<T> const &lhs, intrusive_ptr<T> const &rhs)
{
    return lhs.get() == rhs.get();
}

template<typename T>
MLIO_API
inline constexpr bool
operator!=(intrusive_ptr<T> const &lhs, intrusive_ptr<T> const &rhs)
{
    return lhs.get() != rhs.get();
}

template<typename T>
MLIO_API
inline constexpr bool
operator==(intrusive_ptr<T> const &lhs, std::nullptr_t)
{
    return lhs.get() == nullptr;
}

template<typename T>
MLIO_API
inline constexpr bool
operator!=(intrusive_ptr<T> const &lhs, std::nullptr_t)
{
    return lhs.get() != nullptr;
}

template<typename T>
MLIO_API
inline constexpr bool
operator==(std::nullptr_t, intrusive_ptr<T> const &rhs)
{
    return rhs.get() == nullptr;
}

template<typename T>
MLIO_API
inline constexpr bool
operator!=(std::nullptr_t, intrusive_ptr<T> const &rhs)
{
    return rhs.get() != nullptr;
}

template<typename T, typename... Args>
MLIO_API
inline intrusive_ptr<T>
make_intrusive(Args &&...args)
{
    return intrusive_ptr<T>{new T{std::forward<Args>(args)...}, true};
}

template<typename T>
MLIO_API
inline intrusive_ptr<T>
wrap_intrusive(T *t, bool inc_ref = true)
{
    return intrusive_ptr<T>{t, inc_ref};
}

}  // namespace v1
}  // namespace mlio

namespace std {

template<typename T>
struct MLIO_API hash<mlio::intrusive_ptr<T>> {
    inline size_t
    operator()(mlio::intrusive_ptr<T> const &ptr) const noexcept
    {
        return hash<T *>{}(ptr.get());
    }
};

}  // namespace std
