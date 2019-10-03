#pragma once

#include <string_view>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {

MLIO_API std::string_view
trim(std::string_view s) noexcept;

}  // namespace v1
}  // namespace mlio
