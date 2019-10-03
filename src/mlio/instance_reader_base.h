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

#include "mlio/instance.h"
#include "mlio/instance_reader.h"
#include "mlio/optional.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class instance_reader_base : public instance_reader {
public:
    stdx::optional<instance>
    read_instance() final;

    stdx::optional<instance> const &
    peek_instance() final;

private:
    virtual stdx::optional<instance>
    read_instance_core() = 0;

private:
    stdx::optional<instance> peeked_instance_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
