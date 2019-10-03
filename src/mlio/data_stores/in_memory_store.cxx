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

#include "mlio/data_stores/in_memory_store.h"

#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mlio/logger.h"
#include "mlio/not_supported_error.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/memory_input_stream.h"

namespace mlio {
inline namespace v1 {

in_memory_store::
in_memory_store(memory_slice chunk, compression cmp)
    : chunk_{std::move(chunk)}, compression_{cmp}
{
    if (compression_ == compression::infer) {
        throw not_supported_error{
            "The in-memory store does not support inferring compression."};
    }

    auto *ptr = static_cast<void const *>(chunk_.data());
    id_ = fmt::format("{0:p}+{1:#04x}", ptr, chunk_.size());
}

intrusive_ptr<input_stream>
in_memory_store::
open_read() const
{
    logger::info("The in-memory store '{0}' is being opened.", id_);

    auto strm = make_intrusive<memory_input_stream>(chunk_);

    if (compression_ == compression::none) {
        return std::move(strm);
    }
    return make_inflate_stream(std::move(strm), compression_);
}

std::string
in_memory_store::
repr() const
{
    auto *ptr = static_cast<void const *>(chunk_.data());

    return fmt::format("<in_memory_store address={0:p} size={1:#04x} compression='{2}'>",
                       ptr, chunk_.size(), compression_);
}

}  // namespace v1
}  // namespace mlio
