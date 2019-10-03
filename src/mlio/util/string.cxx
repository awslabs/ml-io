#include "mlio/util/string.h"

#include <algorithm>
#include <cctype>
#include <iterator>

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

template<typename It>
inline auto
find_first_non_space(It first, It last)
{
    auto pos = std::find_if_not(first, last, [](int ch)
    {
        return std::isspace(ch);
    });

    return static_cast<stdx::string_view::size_type>(pos - first);
}

inline void
ltrim(stdx::string_view &s) noexcept
{
    auto offset = find_first_non_space(s.cbegin(), s.cend());

    s.remove_prefix(offset);
}

inline void
rtrim(stdx::string_view &s) noexcept
{
    auto offset = find_first_non_space(s.crbegin(), s.crend());

    s.remove_suffix(offset);
}

}  // namespace
}  // namespace detail

stdx::string_view
trim(stdx::string_view s) noexcept
{
    detail::ltrim(s);
    detail::rtrim(s);

    return s;
}

}  // namespace v1
}  // namespace mlio
