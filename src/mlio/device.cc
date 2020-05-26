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

#include "mlio/device.h"

#include <string>

#include <fmt/format.h>
#include <fmt/ostream.h>

namespace mlio {
inline namespace abi_v1 {

Device_kind Device_kind::cpu() noexcept
{
    static std::string name = "CPU";

    return Device_kind{name};
}

std::string Device_kind::repr() const
{
    return fmt::format("<Device_kind name='{0}'>", name_);
}

std::string Device::repr() const
{
    return fmt::format("<Device kind={0} id={1}>", kind_, id_);
}

}  // namespace abi_v1
}  // namespace mlio
