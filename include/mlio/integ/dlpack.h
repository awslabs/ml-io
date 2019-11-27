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

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

struct DLManagedTensor;

namespace mlio {
inline namespace v1 {

/// Wraps the specified @ref tensor as a DLManagedTensor.
MLIO_API
::DLManagedTensor *
as_dlpack(tensor &tsr, std::size_t version);

MLIO_API
intrusive_ptr<tensor>
as_tensor(::DLManagedTensor *mt, std::size_t version);

}  // namespace v1
}  // namespace mlio
