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

#include <iconv.h>

#include "mlio/span.h"
#include "mlio/text_encoding.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

enum class Iconv_status { ok, incomplete_char, leftover };

class Iconv_desc {
public:
    explicit Iconv_desc(Text_encoding &&encoding);

    Iconv_desc(const Iconv_desc &) = delete;

    Iconv_desc &operator=(const Iconv_desc &) = delete;

    Iconv_desc(Iconv_desc &&) = delete;

    Iconv_desc &operator=(Iconv_desc &&) = delete;

    ~Iconv_desc();

    Iconv_status convert(Memory_span &inp, Mutable_memory_span &out);

    const Text_encoding &encoding() const noexcept
    {
        return encoding_;
    }

private:
    Text_encoding encoding_;
    ::iconv_t desc_;
};

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
