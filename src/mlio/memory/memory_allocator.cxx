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

#include "mlio/memory/memory_allocator.h"

#include <utility>

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

std::unique_ptr<memory_allocator> memory_allocator_{};

}  // namespace
}  // namespace detail

memory_allocator::~memory_allocator() = default;

memory_allocator &
get_memory_allocator() noexcept
{
    return *detail::memory_allocator_;
}

void
set_memory_allocator(std::unique_ptr<memory_allocator> &&alloc) noexcept
{
    detail::memory_allocator_ = std::move(alloc);
}

}  // namespace v1
}  // namespace mlio
