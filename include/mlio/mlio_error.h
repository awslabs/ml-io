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

#include <stdexcept>

#include "mlio/config.h"

namespace mlio {
inline namespace abi_v1 {

class MLIO_API Mlio_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

    Mlio_error(const Mlio_error &) = default;

    Mlio_error &operator=(const Mlio_error &) = default;

    Mlio_error(Mlio_error &&) = default;

    Mlio_error &operator=(Mlio_error &&) = default;

    ~Mlio_error() override;
};

}  // namespace abi_v1
}  // namespace mlio
