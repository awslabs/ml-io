#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

#include "mlio/config.h"
#include "mlio/type_traits.h"

namespace mlio {
inline namespace v1 {
namespace stdx {

template<typename Cont>
inline constexpr std::ptrdiff_t
ssize(Cont const &cont) noexcept
{
    return static_cast<std::ptrdiff_t>(cont.size());
}

}  // namespace stdx

namespace detail {

template<typename T, typename U>
struct is_same_signedness : public stdx::bool_constant<
    std::is_signed<T>::value == std::is_signed<U>::value> {};

}  // namespace detail

MLIO_API
inline constexpr std::size_t
as_size(std::ptrdiff_t d) noexcept
{
    return static_cast<std::size_t>(d);
}

MLIO_API
inline constexpr std::ptrdiff_t
as_ssize(std::size_t s) noexcept
{
    return static_cast<std::ptrdiff_t>(s);
}

template<typename T, typename U>
MLIO_API
inline constexpr T
narrow_cast(U &&u) noexcept
{
    return static_cast<T>(std::forward<U>(u));
}

template<typename T, typename U>
MLIO_API
inline constexpr bool
try_narrow(U u, T &t) noexcept
{
    t = narrow_cast<T>(u);

    if (static_cast<U>(t) != u) {
        return false;
    }
    if (!detail::is_same_signedness<T, U>::value && ((t < T{}) != (u < U{}))) {
        return false;
    }
    return true;
}

}  // namespace v1
}  // namespace mlio
