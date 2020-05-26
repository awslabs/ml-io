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

#include "mlio/memory/file_backed_memory_allocator.h"

#include <algorithm>
#include <utility>

#include "mlio/detail/system_info.h"
#include "mlio/logger.h"
#include "mlio/memory/file_backed_memory_block.h"
#include "mlio/memory/heap_memory_block.h"
#include "mlio/memory/memory_block.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

// This class internally holds a Heap_memory_block once initialized and
// migrates to a File_backed_memory_block if it gets resized and the new
// size exceeds the specified threshold.
class Hybrid_memory_block final : public Mutable_memory_block {
public:
    explicit Hybrid_memory_block(size_type size, size_type oversize_threshold);

    void resize(size_type size) final;

    pointer data() noexcept final
    {
        return inner_->data();
    }

    const_pointer data() const noexcept final
    {
        return inner_->data();
    }

    size_type size() const noexcept final
    {
        return inner_->size();
    }

    bool resizable() const noexcept final
    {
        return true;
    }

private:
    Intrusive_ptr<Mutable_memory_block> inner_;
    size_type oversize_threshold_;
    bool file_backed_{};
};

Hybrid_memory_block::Hybrid_memory_block(size_type size, size_type oversize_threshold)
    : oversize_threshold_{oversize_threshold}
{
    inner_ = make_intrusive<Heap_memory_block>(size);
}

void Hybrid_memory_block::resize(size_type size)
{
    // If the requested size exceeds the threshold, we move the data
    // from the heap to a file-backed block. Note that the reverse is
    // not true. Once we have a file-backed memory block there is no
    // need to move back to the heap; once initialized accessing a
    // file-backed memory region has no extra latency.
    if (!file_backed_ && inner_->size() > oversize_threshold_) {
        logger::debug(
            "The data is being moved from heap to file-backed memory block. Old size was {0:n} byte(s); new size is {1:n} bytes.",
            inner_->size(),
            size);

        auto block = make_intrusive<File_backed_memory_block>(size);

        std::copy(inner_->begin(), inner_->end(), block->begin());

        inner_ = std::move(block);

        file_backed_ = true;
    }
    else {
        inner_->resize(size);
    }
}

std::size_t default_oversize_threshold() noexcept
{
    constexpr std::size_t max_default_threshold = 0x2000'0000;  // 512 MiB

    std::size_t total_ram = get_total_ram();
    if (total_ram == 0) {
        return max_default_threshold;
    }
    std::size_t threshold = std::min(total_ram >> 2, max_default_threshold);

    logger::debug("The default oversize threshold is {0:n} bytes.", threshold);

    return threshold;
}

}  // namespace
}  // namespace detail

using mlio::detail::Hybrid_memory_block;

File_backed_memory_allocator::File_backed_memory_allocator(std::size_t oversize_threshold) noexcept
    : oversize_threshold_{oversize_threshold}
{
    if (oversize_threshold_ == 0) {
        oversize_threshold_ = detail::default_oversize_threshold();
    }
}

Intrusive_ptr<Mutable_memory_block> File_backed_memory_allocator::allocate(std::size_t size)
{
    if (size > oversize_threshold_) {
        return make_intrusive<File_backed_memory_block>(size);
    }
    return make_intrusive<Hybrid_memory_block>(size, oversize_threshold_);
}

}  // namespace abi_v1
}  // namespace mlio
