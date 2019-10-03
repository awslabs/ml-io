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

#include <type_traits>
#include <utility>

#include "mlio/config.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup memory Memory
/// @{

/// Represents a slice of a @ref memory_block.
///
/// Unlike a span a slice shares the ownership of a memory block;
/// meaning the block will be kept alive until all slices referencing
/// it get destructed.
class MLIO_API memory_slice {
public:
    memory_slice() noexcept = default;

    template<typename T>
    // NOLINTNEXTLINE(google-explicit-constructor)
    memory_slice(intrusive_ptr<T> blk) noexcept
        : block_{std::move(blk)}, beg_{block_->begin()}, end_{block_->end()}
    {
        static_assert(std::is_base_of<memory_block, T>::value,
                      "T must derive from memory_block.");
    }

public:
    memory_block::iterator
    begin() const noexcept
    {
        return beg_;
    }

    memory_block::iterator
    end() const noexcept
    {
        return end_;
    }

    memory_block::iterator
    cbegin() const noexcept
    {
        return beg_;
    }

    memory_block::iterator
    cend() const noexcept
    {
        return end_;
    }

public:
    memory_block::pointer
    data() const noexcept
    {
        return beg_ != end_ ? std::addressof(*beg_) : nullptr;
    }

    memory_block::size_type
    size() const noexcept
    {
        return static_cast<memory_block::size_type>(end_ - beg_);
    }

    bool
    empty() const noexcept
    {
        return beg_ == end_;
    }

public:
    memory_slice
    subslice(memory_block::size_type offset) const &
    {
        return subslice(beg_ + as_ssize(offset), end_);
    }

    memory_slice &&
    subslice(memory_block::size_type offset) &&
    {
        return std::move(*this).subslice(beg_ + as_ssize(offset), end_);
    }

    memory_slice
    subslice(memory_block::size_type offset, memory_block::size_type count) const &
    {
        return subslice(beg_ + as_ssize(offset),
                        beg_ + as_ssize(offset + count));
    }

    memory_slice &&
    subslice(memory_block::size_type offset, memory_block::size_type count) &&
    {
        return std::move(*this).subslice(beg_ + as_ssize(offset),
                                         beg_ + as_ssize(offset + count));
    }

    memory_slice
    subslice(memory_block::iterator first) const &
    {
        return subslice(first, end_);
    }

    memory_slice &&
    subslice(memory_block::iterator first) &&
    {
        return std::move(*this).subslice(first, end_);
    }

    memory_slice
    subslice(memory_block::iterator first, memory_block::iterator last) const &
    {
        validate_range(first, last);

        memory_slice cpy = *this;

        cpy.beg_ = first;
        cpy.end_ = last;

        return cpy;
    }

    memory_slice &&
    subslice(memory_block::iterator first, memory_block::iterator last) &&
    {
        validate_range(first, last);

        beg_ = first;
        end_ = last;

        return std::move(*this);
    }

    memory_slice
    first(memory_block::size_type count) const &
    {
        return subslice(beg_, beg_ + as_ssize(count));
    }

    memory_slice &&
    first(memory_block::size_type count) &&
    {
        return std::move(*this).subslice(beg_, beg_ + as_ssize(count));
    }

    memory_slice
    first(memory_block::iterator to) const &
    {
        return subslice(beg_, to);
    }

    memory_slice &&
    first(memory_block::iterator to) &&
    {
        return std::move(*this).subslice(beg_, to);
    }

    memory_slice
    last(memory_block::size_type count) const &
    {
        return subslice(end_ - as_ssize(count), end_);
    }

    memory_slice &&
    last(memory_block::size_type count) &&
    {
        return std::move(*this).subslice(end_ - as_ssize(count), end_);
    }

    memory_slice
    last(memory_block::iterator from) const &
    {
        return subslice(from, end_);
    }

    memory_slice &&
    last(memory_block::iterator from) &&
    {
        return std::move(*this).subslice(from, end_);
    }

private:
    void
    validate_range(memory_block::iterator first,
                   memory_block::iterator last) const;

private:
    intrusive_ptr<memory_block> block_{};
    memory_block::iterator beg_{};
    memory_block::iterator end_{};
};

template<typename T>
MLIO_API
inline stdx::span<T>
as_span(memory_slice const &ms) noexcept
{
    return as_span<T>(memory_span{ms});
}

/// @}

}  // namespace v1
}  // namespace mlio
