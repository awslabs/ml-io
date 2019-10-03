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

#include "mlio/device.h"

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace mlio {
inline namespace v1 {

device_kind
device_kind::
cpu() noexcept
{
    static std::string name = "CPU";

    return device_kind{name};
}

std::string
device_kind::
repr() const
{
    return fmt::format("<device_kind name='{0}'>", name_);
}

std::string
device::
repr() const
{
    return fmt::format("<device kind={0} id={1}>", kind_, id_);
}

}  // namespace v1
}  // namespace mlio
