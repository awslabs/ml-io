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

#include <type_traits>
#include <utility>

#include "mlio/config.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_block.h"
#include "mlio/span.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup memory Memory
/// @{

/// Represents a slice of a @ref Memory_block.
///
/// Unlike a span a slice shares the ownership of a memory block;
/// meaning the block will be kept alive until all slices referencing
/// it get destructed.
class MLIO_API Memory_slice {
public:
    Memory_slice() noexcept = default;

    template<typename T>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Memory_slice(Intrusive_ptr<T> block) noexcept
        : block_{std::move(block)}, beg_{block_->begin()}, end_{block_->end()}
    {
        static_assert(std::is_base_of<Memory_block, T>::value, "T must derive from Memory_block.");
    }

    Memory_block::iterator begin() const noexcept
    {
        return beg_;
    }

    Memory_block::iterator end() const noexcept
    {
        return end_;
    }

    Memory_block::iterator cbegin() const noexcept
    {
        return beg_;
    }

    Memory_block::iterator cend() const noexcept
    {
        return end_;
    }

    Memory_block::pointer data() const noexcept
    {
        if (beg_ == Memory_block::iterator{}) {
            return nullptr;
        }

        return std::addressof(*beg_);
    }

    Memory_block::size_type size() const noexcept
    {
        return static_cast<Memory_block::size_type>(end_ - beg_);
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return beg_ == end_;
    }

    Memory_slice subslice(Memory_block::size_type offset) const &
    {
        return subslice(beg_ + as_ssize(offset), end_);
    }

    Memory_slice &&subslice(Memory_block::size_type offset) &&
    {
        return std::move(*this).subslice(beg_ + as_ssize(offset), end_);
    }

    Memory_slice subslice(Memory_block::size_type offset, Memory_block::size_type count) const &
    {
        return subslice(beg_ + as_ssize(offset), beg_ + as_ssize(offset + count));
    }

    Memory_slice &&subslice(Memory_block::size_type offset, Memory_block::size_type count) &&
    {
        return std::move(*this).subslice(beg_ + as_ssize(offset), beg_ + as_ssize(offset + count));
    }

    Memory_slice subslice(Memory_block::iterator first) const &
    {
        return subslice(first, end_);
    }

    Memory_slice &&subslice(Memory_block::iterator first) &&
    {
        return std::move(*this).subslice(first, end_);
    }

    Memory_slice subslice(Memory_block::iterator first, Memory_block::iterator last) const &
    {
        validate_range(first, last);

        Memory_slice cpy = *this;

        cpy.beg_ = first;
        cpy.end_ = last;

        return cpy;
    }

    Memory_slice &&subslice(Memory_block::iterator first, Memory_block::iterator last) &&
    {
        validate_range(first, last);

        beg_ = first;
        end_ = last;

        return std::move(*this);
    }

    Memory_slice first(Memory_block::size_type count) const &
    {
        return subslice(beg_, beg_ + as_ssize(count));
    }

    Memory_slice &&first(Memory_block::size_type count) &&
    {
        return std::move(*this).subslice(beg_, beg_ + as_ssize(count));
    }

    Memory_slice first(Memory_block::iterator to) const &
    {
        return subslice(beg_, to);
    }

    Memory_slice &&first(Memory_block::iterator to) &&
    {
        return std::move(*this).subslice(beg_, to);
    }

    Memory_slice last(Memory_block::size_type count) const &
    {
        return subslice(end_ - as_ssize(count), end_);
    }

    Memory_slice &&last(Memory_block::size_type count) &&
    {
        return std::move(*this).subslice(end_ - as_ssize(count), end_);
    }

    Memory_slice last(Memory_block::iterator from) const &
    {
        return subslice(from, end_);
    }

    Memory_slice &&last(Memory_block::iterator from) &&
    {
        return std::move(*this).subslice(from, end_);
    }

private:
    void validate_range(Memory_block::iterator first, Memory_block::iterator last) const;

    Intrusive_ptr<Memory_block> block_{};
    Memory_block::iterator beg_{};
    Memory_block::iterator end_{};
};

template<typename T>
MLIO_API
inline stdx::span<T> as_span(const Memory_slice &slice) noexcept
{
    return as_span<T>(Memory_span{slice});
}

/// @}

}  // namespace abi_v1
}  // namespace mlio
