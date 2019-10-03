#pragma once

#include <thread>
#include <utility>

#ifdef MLIO_PLATFORM_UNIX
#include <csignal>
#include <system_error>

#include <pthread.h>
#endif

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {
namespace detail {

template<typename Func, typename... Args>
std::thread
start_thread(Func &&f, Args &&...args)
{
#ifdef MLIO_PLATFORM_UNIX
    // Block all asynchronous signals on the new thread.

    ::sigset_t mask{}, original_mask{};
    ::sigfillset(&mask);

    int s;

    s = ::pthread_sigmask(SIG_SETMASK, &mask, &original_mask);
    if (s != 0) {
        throw std::system_error{s, std::generic_category()};
    }
#endif

    std::thread t{std::forward<Func>(f), std::forward<Args>(args)...};

#ifdef MLIO_PLATFORM_UNIX
    // Restore the signal mask.

    s = ::pthread_sigmask(SIG_SETMASK, &original_mask, nullptr);
    if (s != 0) {
        throw std::system_error{s, std::generic_category()};
    }
#endif

    return t;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
