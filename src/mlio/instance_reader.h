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

#include "mlio/fwd.h"
#include "mlio/optional.h"

namespace mlio {
inline namespace v1 {
namespace detail {

// Reads data instances from a dataset.
class instance_reader {
public:
    instance_reader() noexcept = default;

    instance_reader(instance_reader const &) = delete;

    instance_reader(instance_reader &&) = delete;

    virtual
   ~instance_reader();

public:
    instance_reader &
    operator=(instance_reader const &) = delete;

    instance_reader &
    operator=(instance_reader &&) = delete;

public:
    virtual stdx::optional<instance>
    read_instance() = 0;

    virtual stdx::optional<instance> const &
    peek_instance() = 0;

    virtual void
    reset() noexcept = 0;
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
