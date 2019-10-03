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

#include <zlib.h>

#include "mlio/span.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class zlib_inflater {
public:
    explicit
    zlib_inflater();

    zlib_inflater(zlib_inflater const &) = delete;

    zlib_inflater(zlib_inflater &&) = delete;

   ~zlib_inflater();

public:
    zlib_inflater &
    operator=(zlib_inflater const &) = delete;

    zlib_inflater &
    operator=(zlib_inflater &&) = delete;

public:
    void
    inflate(memory_span &inp, mutable_memory_span &out);

    bool
    eof() const noexcept
    {
        return state_ == Z_STREAM_END;
    }

private:
    void
    validate_state();

private:
    ::z_stream stream_{};
    int state_ = Z_STREAM_END;
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
