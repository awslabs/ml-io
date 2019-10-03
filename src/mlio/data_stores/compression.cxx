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

#include "mlio/data_stores/compression.h"

#include <stdexcept>
#include <utility>

#include "mlio/not_supported_error.h"
#include "mlio/streams/gzip_inflate_stream.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {

intrusive_ptr<input_stream>
make_inflate_stream(intrusive_ptr<input_stream> &&strm, compression cmp)
{
    switch (cmp) {
    case compression::none:
    case compression::infer:
        return std::move(strm);

    case compression::gzip:
        return make_intrusive<gzip_inflate_stream>(std::move(strm));

    case compression::bzip2:
    case compression::zip:
        throw not_supported_error{
            "bzip2 and zip compressions are not supported yet."};
    }

    throw std::invalid_argument{"The specified compression is not supported."};
}

}  // namespace v1
}  // namespace mlio
