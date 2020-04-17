#pragma once

#include <cstddef>
#include <string_view>

#include "mlio/config.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {

MLIO_API
std::string_view
trim(std::string_view s) noexcept;

MLIO_API
bool
is_only_whitespace(std::string_view s) noexcept;

MLIO_API
inline std::string_view
as_string_view(stdx::span<std::byte const> bits) noexcept
{
    auto chrs = as_span<char const>(bits);

    return std::string_view{chrs.data(), chrs.size()};
}

}  // namespace v1
}  // namespace mlio
