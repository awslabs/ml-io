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
inline namespace v1 {

/// @addtogroup streams Streams
/// @{

class MLIO_API stream_error : public mlio_error {
public:
    using mlio_error::mlio_error;

public:
    stream_error(stream_error const &) = default;

    stream_error(stream_error &&) = default;

    ~stream_error() override;

public:
    stream_error &operator=(stream_error const &) = default;

    stream_error &operator=(stream_error &&) = default;
};

class MLIO_API inflate_error : public stream_error {
public:
    using stream_error::stream_error;

public:
    inflate_error(inflate_error const &) = default;

    inflate_error(inflate_error &&) = default;

    ~inflate_error() override;

public:
    inflate_error &operator=(inflate_error const &) = default;

    inflate_error &operator=(inflate_error &&) = default;
};

/// @}

}  // namespace v1
}  // namespace mlio
