/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
inline namespace v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Specifies the compression type of a data store.
enum class compression {
    none,
    infer,
    gzip,
    bzip2,
    zip
};

/// Constructs a new inflate stream by wrapping the specified input
/// stream.
MLIO_API
intrusive_ptr<input_stream>
make_inflate_stream(intrusive_ptr<input_stream> &&strm, compression cmp);

MLIO_API
inline std::ostream &
operator<<(std::ostream &strm, compression cmp)
{
    switch (cmp) {
    case compression::none:
        strm << "none";
        break;
    case compression::infer:
        strm << "infer";
        break;
    case compression::gzip:
        strm << "gzip";
        break;
    case compression::bzip2:
        strm << "bzip2";
        break;
    case compression::zip:
        strm << "zip";
        break;
    }
    return strm;
}

/// @}

}  // namespace v1
}  // namespace mlio
