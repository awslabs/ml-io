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

#include "mlio/text_encoding.h"

namespace mlio {
inline namespace v1 {

text_encoding const text_encoding::ascii_latin1{"8859_1"};    // NOLINT(cert-err58-cpp)
text_encoding const text_encoding::utf8        {"UTF-8"};     // NOLINT(cert-err58-cpp)
text_encoding const text_encoding::utf16       {"UTF-16"};    // NOLINT(cert-err58-cpp)
text_encoding const text_encoding::utf16_le    {"UTF-16LE"};  // NOLINT(cert-err58-cpp)
text_encoding const text_encoding::utf16_be    {"UTF-16BE"};  // NOLINT(cert-err58-cpp)
text_encoding const text_encoding::utf32       {"UTF-32"};    // NOLINT(cert-err58-cpp)
text_encoding const text_encoding::utf32_le    {"UTF-32LE"};  // NOLINT(cert-err58-cpp)
text_encoding const text_encoding::utf32_be    {"UTF-32BE"};  // NOLINT(cert-err58-cpp)

}  // namespace v1
}  // namespace mlio
