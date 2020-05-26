/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 3.0 (the "License"). You
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

#include <iostream>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Specifies the compression type of a data store.
enum class Compression { none, infer, gzip, bzip2, zip };

/// Constructs a new inflate stream by wrapping the specified input
/// stream.
MLIO_API
Intrusive_ptr<Input_stream>
make_inflate_stream(Intrusive_ptr<Input_stream> &&stream, Compression compression);

MLIO_API
inline std::ostream &operator<<(std::ostream &s, Compression compression)
{
    switch (compression) {
    case Compression::none:
        s << "none";
        break;
    case Compression::infer:
        s << "infer";
        break;
    case Compression::gzip:
        s << "gzip";
        break;
    case Compression::bzip2:
        s << "bzip2";
        break;
    case Compression::zip:
        s << "zip";
        break;
    }
    return s;
}

/// @}

}  // namespace abi_v1
}  // namespace mlio
