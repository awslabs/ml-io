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

#include <iconv.h>

#include "mlio/span.h"
#include "mlio/text_encoding.h"

namespace mlio {
inline namespace v1 {
namespace detail {

enum class iconv_status {
    ok,
    incomplete_char,
    leftover
};

class iconv_desc {
public:
    explicit
    iconv_desc(text_encoding &&enc);

    iconv_desc(iconv_desc const &) = delete;

    iconv_desc(iconv_desc &&) = delete;

   ~iconv_desc();

public:
    iconv_desc &
    operator=(iconv_desc const &) = delete;

    iconv_desc &
    operator=(iconv_desc &&) = delete;

public:
    iconv_status
    convert(memory_span &inp, mutable_memory_span &out);

public:
    text_encoding const &
    encoding() const noexcept
    {
        return encoding_;
    }

private:
    text_encoding encoding_;
    ::iconv_t desc_;
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
