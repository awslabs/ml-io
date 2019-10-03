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

#include "mlio/logger.h"

#include <functional>
#include <type_traits>
#include <utility>

#include <fmt/format.h>

namespace mlio {
inline namespace v1 {
namespace logger {
namespace {

log_message_handler msg_handler_{};

log_level level_{log_level::warning};

}  // namespace

namespace detail {

void
handle_message(log_level lvl, stdx::string_view msg) noexcept
{
    if (msg_handler_ != nullptr) {
        try {
            msg_handler_(lvl, msg);
        }
        catch (...)
        {}
    }
}

void
handle_message(log_level lvl, stdx::string_view fmt, fmt::format_args args) noexcept
{
    fmt::memory_buffer buf;
    try {
        fmt::vformat_to(buf, fmt, args);
    }
    catch (...)
    {
        return;
    }

    handle_message(lvl, stdx::string_view(buf.data(), buf.size()));
}

}  // namespace detail

bool
is_enabled_for(log_level lvl) noexcept
{
    return lvl <= level_;
}

}  // namespace logger

log_message_handler
set_log_message_handler(log_message_handler hdl) noexcept
{
    return std::exchange(logger::msg_handler_, std::move(hdl));
}

void
set_log_level(log_level lvl) noexcept
{
    logger::level_ = lvl;
}

}  // namespace v1
}  // namespace mlio
