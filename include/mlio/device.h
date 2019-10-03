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
#include <functional>
#include <iostream>

#include "mlio/config.h"
#include "mlio/string_view.h"
#include "mlio/util/hash.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup tensors Tensors
/// @{

/// Represents a device kind that has data processing capabilities such
/// as CPU or CUDA.
class MLIO_API device_kind {
public:
    /// A convenience function for the CPU device kind.
    static device_kind
    cpu() noexcept;

public:
    /// @param name
    ///     The name can be any arbitrary string, but it must be unique
    ///     among device kinds.
    explicit constexpr
    device_kind(stdx::string_view name) noexcept
      : name_{name}
    {}

public:
    std::string
    repr() const;

public:
    constexpr stdx::string_view
    name() const noexcept
    {
        return name_;
    }

private:
    stdx::string_view name_;
};

MLIO_API
inline bool
operator==(device_kind const &lhs, device_kind const &rhs) noexcept
{
    return lhs.name() == rhs.name();
}

MLIO_API
inline bool
operator!=(device_kind const &lhs, device_kind const &rhs) noexcept
{
    return lhs.name() != rhs.name();
}

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, device_kind const &kind)
{
    return strm << kind.repr();
}

/// Represents a particular data processing unit on the host system.
class MLIO_API device {
public:
    explicit constexpr
    device(device_kind kind, std::size_t id = 0) noexcept
        : kind_{kind}, id_{id}
    {}

public:
    std::string
    repr() const;

public:
    constexpr device_kind
    kind() const noexcept
    {
        return kind_;
    }

    constexpr std::size_t
    id() const noexcept
    {
        return id_;
    }

private:
    device_kind kind_;
    std::size_t id_;
};

MLIO_API
inline bool
operator==(device const &lhs, device const &rhs) noexcept
{
    return lhs.kind() == rhs.kind() && lhs.id() == rhs.id();
}

MLIO_API
inline bool
operator!=(device const &lhs, device const &rhs) noexcept
{
    return lhs.kind() != rhs.kind() || lhs.id() != rhs.id();
}

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, device const &dev)
{
    return strm << dev.repr();
}

/// @}

}  // namespace v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::device_kind> {
    inline size_t
    operator()(mlio::device_kind const &kind) const noexcept
    {
        return hash<mlio::stdx::string_view>{}(kind.name());
    }
};

template<>
struct MLIO_API hash<mlio::device> {
    inline size_t
    operator()(mlio::device const &dev) const noexcept
    {
        size_t seed = 0;

        mlio::detail::hash_combine(seed, dev.kind());
        mlio::detail::hash_combine(seed, dev.id());

        return seed;
    }
};

}  // namespace std
