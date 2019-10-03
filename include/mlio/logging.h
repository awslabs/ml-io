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

#include <functional>

#include "mlio/config.h"
#include "mlio/string_view.h"

namespace mlio {
inline namespace v1 {

enum class log_level {
    off,
    warning,
    info,
    debug
};

/// Represents a delegate function that handles logging on behalf of
/// the library.
using log_message_handler = std::function<void(log_level, stdx::string_view)>;

/// Sets the logging handler.
///
/// The passsed handler can be called by multiple threads
/// simultaneously; therefore it should be thread-safe.
MLIO_API
log_message_handler
set_log_message_handler(log_message_handler hdl) noexcept;

MLIO_API
void
set_log_level(log_level lvl) noexcept;

}  // namespace v1
}  // namespace mlio
