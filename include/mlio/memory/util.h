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
#include <memory>
#include <type_traits>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup memory Memory
/// @{

MLIO_API
intrusive_ptr<mutable_memory_block>
resize_memory_block(intrusive_ptr<mutable_memory_block> &blk, std::size_t size);

template<typename T>
MLIO_API
std::unique_ptr<T>
wrap_unique(T *ptr) noexcept
{
    static_assert(!std::is_array<T>::value, "T must be a non-array type.");

    return std::unique_ptr<T>(ptr);
}

/// @}

}  // namespace v1
}  // namespace mlio
