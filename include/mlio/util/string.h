#pragma once

#include <string_view>
#include <unordered_set>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {

MLIO_API std::string_view
trim(std::string_view s) noexcept;

MLIO_API bool
is_only_whitespace(std::string_view s) noexcept;

MLIO_API bool
matches(std::string_view s,
        std::unordered_set<std::string> const &match_values) noexcept;

}  // namespace v1
}  // namespace mlio
