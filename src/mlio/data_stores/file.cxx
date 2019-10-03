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

#include "mlio/data_stores/file.h"

#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mlio/data_stores/detail/file_util.h"
#include "mlio/detail/pathname.h"
#include "mlio/logger.h"
#include "mlio/memory/file_mapped_memory_block.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/file_input_stream.h"
#include "mlio/streams/memory_input_stream.h"

namespace mlio {
inline namespace v1 {

file::
file(std::string pathname, bool mmap, compression cmp)
    : pathname_{std::move(pathname)}, mmap_{mmap}, compression_{cmp}
{
    detail::validate_file_pathname(pathname_);

    if (compression_ == compression::infer) {
        compression_ = detail::infer_compression(pathname_);
    }
}

intrusive_ptr<input_stream>
file::
open_read() const
{
    logger::info("The file '{0}' is being opened.", pathname_);

    intrusive_ptr<input_stream> strm;
    if (mmap_) {
        strm = make_intrusive<memory_input_stream>(
            make_intrusive<file_mapped_memory_block>(pathname_));
    } else {
        strm = make_intrusive<file_input_stream>(pathname_);
    }

    if (compression_ == compression::none) {
        return strm;
    }
    return make_inflate_stream(std::move(strm), compression_);
}

std::string
file::
repr() const
{
    return fmt::format("<file pathname='{0}' memory_mapped='{1}' compression='{2}'>",
                       pathname_, mmap_, compression_);
}

}  // namespace v1
}  // namespace mlio
