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

#include "mlio/record_readers/record_error.h"

namespace mlio {
inline namespace abi_v1 {

Record_error::~Record_error() = default;

Corrupt_record_error::~Corrupt_record_error() = default;

Corrupt_header_error::~Corrupt_header_error() = default;

Corrupt_footer_error::~Corrupt_footer_error() = default;

Record_too_large_error::~Record_too_large_error() = default;

}  // namespace abi_v1
}  // namespace mlio
