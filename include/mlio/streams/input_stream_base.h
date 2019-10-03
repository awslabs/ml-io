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

#include "mlio/config.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup streams Streams
/// @{

class MLIO_API input_stream_base : public input_stream {
public:
    using input_stream::read;

    memory_slice
    read(std::size_t size) override;

    void
    seek(std::size_t position) override;

public:
    std::size_t
    size() const override;

    std::size_t
    position() const override;

    bool
    seekable() const noexcept override
    {
        return false;
    }

    bool
    supports_zero_copy() const noexcept override
    {
        return false;
    }
};

/// @}

}  // namespace v1
}  // namespace mlio
