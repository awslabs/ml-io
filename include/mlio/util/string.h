#pragma once

#include "mlio/config.h"
#include "mlio/string_view.h"

namespace mlio {
inline namespace v1 {

MLIO_API
stdx::string_view
trim(stdx::string_view s) noexcept;

}  // namespace v1
}  // namespace mlio
