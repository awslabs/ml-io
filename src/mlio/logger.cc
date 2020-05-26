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

#include "mlio/logger.h"

#include <functional>
#include <type_traits>
#include <utility>

#include <fmt/format.h>

namespace mlio {
inline namespace abi_v1 {
namespace logger {
namespace {

Log_message_handler msg_handler_{};

Log_level level_{Log_level::warning};

}  // namespace

namespace detail {

void handle_message(Log_level level, std::string_view msg) noexcept
{
    if (msg_handler_ != nullptr) {
        try {
            msg_handler_(level, msg);
        }
        catch (...) {
        }
    }
}

void handle_message(Log_level level, std::string_view format, fmt::format_args args) noexcept
{
    fmt::memory_buffer buf;
    try {
        fmt::vformat_to(buf, format, args);
    }
    catch (...) {
        return;
    }

    handle_message(level, std::string_view(buf.data(), buf.size()));
}

}  // namespace detail

bool is_enabled_for(Log_level level) noexcept
{
    return level <= level_;
}

}  // namespace logger

Log_message_handler set_log_message_handler(Log_message_handler handler) noexcept
{
    return std::exchange(logger::msg_handler_, std::move(handler));
}

void set_log_level(Log_level level) noexcept
{
    logger::level_ = level;
}

}  // namespace abi_v1
}  // namespace mlio
