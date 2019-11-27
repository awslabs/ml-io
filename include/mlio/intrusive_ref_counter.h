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

#include <atomic>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {

template<typename T>
class MLIO_API intrusive_ref_counter {
public:
    intrusive_ref_counter() noexcept = default;

    intrusive_ref_counter(intrusive_ref_counter const &) noexcept
    {}

    intrusive_ref_counter(intrusive_ref_counter &&) noexcept
    {}

protected:
    ~intrusive_ref_counter() = default;

public:
    intrusive_ref_counter &
    // NOLINTNEXTLINE(cert-oop54-cpp)
    operator=(intrusive_ref_counter const &) noexcept
    {
        return *this;
    }

    intrusive_ref_counter &
    operator=(intrusive_ref_counter &&) noexcept
    {
        return *this;
    }

public:
    std::size_t
    use_count() const noexcept
    {
        return ref_count_.load();
    }

public:
    MLIO_API
    friend inline void
    intrusive_ptr_inc_ref(intrusive_ref_counter const *ptr) noexcept
    {
        ptr->ref_count_.fetch_add(1, std::memory_order_acq_rel);
    }

    MLIO_API
    friend inline void
    intrusive_ptr_dec_ref(intrusive_ref_counter const *ptr) noexcept
    {
        if (ptr->ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            delete static_cast<T const *>(ptr);
        }
    }

private:
    mutable std::atomic_size_t ref_count_{0};
};

}  // namespace v1
}  // namespace mlio
