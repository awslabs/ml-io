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

#include "mlio/data_reader_error.h"

namespace mlio {
inline namespace abi_v1 {

Data_reader_error::~Data_reader_error() = default;

Schema_error::~Schema_error() = default;

Invalid_instance_error::~Invalid_instance_error() = default;

}  // namespace abi_v1
}  // namespace mlio
