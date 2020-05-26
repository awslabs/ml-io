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

#include <cstddef>
#include <functional>
#include <iostream>
#include <string_view>

#include "mlio/config.h"
#include "mlio/util/hash.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup tensors Tensors
/// @{

/// Represents a Device kind that has data processing capabilities such
/// as CPU or CUDA.
class MLIO_API Device_kind {
public:
    /// A convenience function for the CPU Device kind.
    static Device_kind cpu() noexcept;

    /// @param name
    ///     The name can be any arbitrary string, but it must be unique
    ///     among Device kinds.
    explicit constexpr Device_kind(std::string_view name) noexcept : name_{name}
    {}

    std::string repr() const;

    constexpr std::string_view name() const noexcept
    {
        return name_;
    }

private:
    std::string_view name_;
};

MLIO_API
inline bool operator==(const Device_kind &lhs, const Device_kind &rhs) noexcept
{
    return lhs.name() == rhs.name();
}

MLIO_API
inline bool operator!=(const Device_kind &lhs, const Device_kind &rhs) noexcept
{
    return lhs.name() != rhs.name();
}

MLIO_API
inline std::ostream &operator<<(std::ostream &s, const Device_kind &kind)
{
    return s << kind.repr();
}

/// Represents a particular data processing unit on the host system.
class MLIO_API Device {
public:
    explicit constexpr Device(Device_kind kind, std::size_t id = 0) noexcept : kind_{kind}, id_{id}
    {}

    std::string repr() const;

    constexpr Device_kind kind() const noexcept
    {
        return kind_;
    }

    constexpr std::size_t id() const noexcept
    {
        return id_;
    }

private:
    Device_kind kind_;
    std::size_t id_;
};

MLIO_API
inline bool operator==(const Device &lhs, const Device &rhs) noexcept
{
    return lhs.kind() == rhs.kind() && lhs.id() == rhs.id();
}

MLIO_API
inline bool operator!=(const Device &lhs, const Device &rhs) noexcept
{
    return lhs.kind() != rhs.kind() || lhs.id() != rhs.id();
}

MLIO_API
inline std::ostream &operator<<(std::ostream &s, const Device &dev)
{
    return s << dev.repr();
}

/// @}

}  // namespace abi_v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::Device_kind> {
    inline size_t operator()(const mlio::Device_kind &kind) const noexcept
    {
        return hash<string_view>{}(kind.name());
    }
};

template<>
struct MLIO_API hash<mlio::Device> {
    inline size_t operator()(const mlio::Device &dev) const noexcept
    {
        size_t seed = 0;

        mlio::detail::hash_combine(seed, dev.kind());
        mlio::detail::hash_combine(seed, dev.id());

        return seed;
    }
};

}  // namespace std
