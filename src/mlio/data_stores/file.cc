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

#include "mlio/data_stores/file.h"

#include <utility>

#include <fmt/format.h>

#include "mlio/data_stores/detail/util.h"
#include "mlio/detail/path.h"
#include "mlio/logger.h"
#include "mlio/memory/file_mapped_memory_block.h"
#include "mlio/streams/file_input_stream.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/memory_input_stream.h"

namespace mlio {
inline namespace abi_v1 {

File::File(std::string path, bool memory_map, Compression compression)
    : path_{std::move(path)}, memory_map_{memory_map}, compression_{compression}
{
    detail::validate_file_path(path_);

    if (compression_ == Compression::infer) {
        compression_ = detail::infer_compression(path_);
    }
}

Intrusive_ptr<Input_stream> File::open_read() const
{
    logger::info("The file '{0}' is being opened.", path_);

    Intrusive_ptr<Input_stream> stream{};
    if (memory_map_) {
        auto block = make_intrusive<File_mapped_memory_block>(path_);
        stream = make_intrusive<Memory_input_stream>(std::move(block));
    }
    else {
        stream = make_intrusive<File_input_stream>(path_);
    }

    if (compression_ == Compression::none) {
        return stream;
    }
    return make_inflate_stream(std::move(stream), compression_);
}

std::string File::repr() const
{
    return fmt::format(
        "<File path='{0}' memory_map='{1}' compression='{2}'>", path_, memory_map_, compression_);
}

}  // namespace abi_v1
}  // namespace mlio
