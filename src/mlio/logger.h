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

#include <string_view>

#include <fmt/format.h>

#include "mlio/logging.h"  // IWYU pragma: export

namespace mlio {
inline namespace abi_v1 {
namespace logger {
namespace detail {

void handle_message(Log_level level, std::string_view msg) noexcept;

void handle_message(Log_level level, std::string_view format, fmt::format_args args) noexcept;

}  // namespace detail

bool is_enabled_for(Log_level level) noexcept;

inline void log(Log_level level, std::string_view msg) noexcept
{
    if (is_enabled_for(level)) {
        detail::handle_message(level, msg);
    }
}

template<typename... Args>
inline void log(Log_level level, std::string_view format, const Args &... args) noexcept
{
    if (is_enabled_for(level)) {
        detail::handle_message(level, format, fmt::make_format_args(args...));
    }
}

inline void warn(std::string_view msg) noexcept
{
    log(Log_level::warning, msg);
}

template<typename... Args>
inline void warn(std::string_view format, const Args &... args) noexcept
{
    log(Log_level::warning, format, args...);
}

inline void info(std::string_view msg) noexcept
{
    log(Log_level::info, msg);
}

template<typename... Args>
inline void info(std::string_view format, const Args &... args) noexcept
{
    log(Log_level::info, format, args...);
}

inline void debug(std::string_view msg) noexcept
{
    log(Log_level::debug, msg);
}

template<typename... Args>
inline void debug(std::string_view format, const Args &... args) noexcept
{
    log(Log_level::debug, format, args...);
}

}  // namespace logger
}  // namespace abi_v1
}  // namespace mlio
