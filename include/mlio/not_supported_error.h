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

#include <stdexcept>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {

class MLIO_API not_supported_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

public:
    not_supported_error(not_supported_error const &) = default;

    not_supported_error(not_supported_error &&) = default;

   ~not_supported_error() override;

public:
    not_supported_error &
    operator=(not_supported_error const &) = default;

    not_supported_error &
    operator=(not_supported_error &&) = default;
};

}  // namespace v1
}  //namespace mlio
