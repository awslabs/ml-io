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

#include <csignal>
#include <system_error>
#include <thread>
#include <utility>

#include <pthread.h>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {
namespace detail {

template<typename Func, typename... Args>
std::thread
start_thread(Func &&f, Args &&... args)
{
    // Block all asynchronous signals on the new thread.
    ::sigset_t mask{};
    ::sigset_t original_mask{};
    sigfillset(&mask);

    int s = ::pthread_sigmask(SIG_SETMASK, &mask, &original_mask);
    if (s != 0) {
        throw std::system_error{s, std::generic_category()};
    }

    std::thread t{std::forward<Func>(f), std::forward<Args>(args)...};

    // Restore the signal mask.
    s = ::pthread_sigmask(SIG_SETMASK, &original_mask, nullptr);
    if (s != 0) {
        throw std::system_error{s, std::generic_category()};
    }

    return t;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
