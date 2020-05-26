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

#include <zlib.h>

#include "mlio/span.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Zlib_inflater {
public:
    explicit Zlib_inflater();

    Zlib_inflater(const Zlib_inflater &) = delete;

    Zlib_inflater &operator=(const Zlib_inflater &) = delete;

    Zlib_inflater(Zlib_inflater &&) = delete;

    Zlib_inflater &operator=(Zlib_inflater &&) = delete;

    ~Zlib_inflater();

    void inflate(Memory_span &inp, Mutable_memory_span &out);

    bool eof() const noexcept
    {
        return state_ == Z_STREAM_END;
    }

private:
    void validate_state() const;

    ::z_stream stream_{};
    int state_ = Z_STREAM_END;
};

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
