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

#include <fmt/format.h>

#include "mlio/logging.h"  // IWYU pragma: export
#include "mlio/string_view.h"

namespace mlio {
inline namespace v1 {
namespace logger {
namespace detail {

void
handle_message(log_level lvl, stdx::string_view msg) noexcept;

void
handle_message(log_level lvl, stdx::string_view fmt, fmt::format_args args) noexcept;

}  // namespace detail

bool
is_enabled_for(log_level lvl) noexcept;

inline void
log(log_level lvl, stdx::string_view msg) noexcept
{
    if (is_enabled_for(lvl)) {
        detail::handle_message(lvl, msg);
    }
}

template<typename... Args>
inline void
log(log_level lvl, stdx::string_view fmt, Args const &...args) noexcept
{
    if (is_enabled_for(lvl)) {
        detail::handle_message(lvl, fmt, fmt::make_format_args(args...));
    }
}

inline void
warn(stdx::string_view msg) noexcept
{
    log(log_level::warning, msg);
}

template<typename... Args>
inline void
warn(stdx::string_view fmt, Args const &...args) noexcept
{
    log(log_level::warning, fmt, args...);
}

inline void
info(stdx::string_view msg) noexcept
{
    log(log_level::info, msg);
}

template<typename... Args>
inline void
info(stdx::string_view fmt, Args const &...args) noexcept
{
    log(log_level::info, fmt, args...);
}

inline void
debug(stdx::string_view msg) noexcept
{
    log(log_level::debug, msg);
}

template<typename... Args>
inline void
debug(stdx::string_view fmt, Args const &...args) noexcept
{
    log(log_level::debug, fmt, args...);
}

}  // namespace logger
}  // namespace v1
}  // namespace mlio
