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
#include <string>
#include <utility>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {

class MLIO_API text_encoding {
public:
    static text_encoding const ascii_latin1;
    static text_encoding const utf8;
    static text_encoding const utf16;
    static text_encoding const utf16_le;
    static text_encoding const utf16_be;
    static text_encoding const utf32;
    static text_encoding const utf32_le;
    static text_encoding const utf32_be;

public:
    /// @remark
    ///     The specified name should be a valid encoding name that is
    ///     accepted by @e iconv(1).
    explicit
    text_encoding(std::string name)
        : name_{std::move(name)}
    {}

public:
    std::string const &
    name() const noexcept
    {
        return name_;
    }

private:
    std::string name_;
};

MLIO_API
inline bool
operator==(text_encoding const &lhs, text_encoding const &rhs) noexcept
{
    return lhs.name() == rhs.name();
}

MLIO_API
inline bool
operator!=(text_encoding const &lhs, text_encoding const &rhs) noexcept
{
    return lhs.name() != rhs.name();
}

}  // namespace v1
}  // namespace mlio

namespace std {

template<>
struct MLIO_API hash<mlio::text_encoding> {
    inline size_t
    operator()(mlio::text_encoding const &enc) const noexcept
    {
        return hash<string>{}(enc.name());
    }
};

}  // namespace std
