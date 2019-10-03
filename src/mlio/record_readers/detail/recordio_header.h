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

#include <cstddef>
#include <cstdint>

#include "mlio/optional.h"
#include "mlio/record_readers/record.h"
#include "mlio/span.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class recordio_header {
public:
    static constexpr std::size_t alignment = sizeof(std::uint32_t);

public:
    explicit
    recordio_header(std::uint32_t data) noexcept
        : data_{data}
    {}

public:
    record_kind
    get_record_kind() const noexcept
    {
        return static_cast<record_kind>((data_ >> 29U) & 7U);
    }

    std::size_t
    payload_size() const noexcept
    {
        return data_ & ((1U << 29U) - 1U);
    }

    std::size_t
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    size() const noexcept
    {
        return sizeof(std::uint32_t) * 2;
    }

private:
    std::uint32_t data_;
};

stdx::optional<recordio_header>
decode_recordio_header(memory_span bits);

}  // namespace detail
}  // namespace v1
}  // namespace mlio
