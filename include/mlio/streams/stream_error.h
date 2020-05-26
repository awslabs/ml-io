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

#include "mlio/config.h"
#include "mlio/mlio_error.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup streams Streams
/// @{

class MLIO_API Stream_error : public Mlio_error {
public:
    using Mlio_error::Mlio_error;

    Stream_error(const Stream_error &) = default;

    Stream_error &operator=(const Stream_error &) = default;

    Stream_error(Stream_error &&) = default;

    Stream_error &operator=(Stream_error &&) = default;

    ~Stream_error() override;
};

class MLIO_API Inflate_error : public Stream_error {
public:
    using Stream_error::Stream_error;

    Inflate_error(const Inflate_error &) = default;

    Inflate_error &operator=(const Inflate_error &) = default;

    Inflate_error(Inflate_error &&) = default;

    Inflate_error &operator=(Inflate_error &&) = default;

    ~Inflate_error() override;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
