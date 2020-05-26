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

#include "mlio/data_stores/in_memory_store.h"

#include <utility>

#include <fmt/format.h>

#include "mlio/logger.h"
#include "mlio/not_supported_error.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/memory_input_stream.h"

namespace mlio {
inline namespace abi_v1 {

In_memory_store::In_memory_store(Memory_slice chunk, Compression compression)
    : chunk_{std::move(chunk)}, compression_{compression}
{
    if (compression_ == Compression::infer) {
        throw Not_supported_error{"The in-memory store does not support inferring compression."};
    }
}

Intrusive_ptr<Input_stream> In_memory_store::open_read() const
{
    if (logger::is_enabled_for(Log_level::info)) {
        logger::info("The in-memory store '{0}' is being opened.", id());
    }

    auto stream = make_intrusive<Memory_input_stream>(chunk_);

    if (compression_ == Compression::none) {
        return std::move(stream);
    }
    return make_inflate_stream(std::move(stream), compression_);
}

const std::string &In_memory_store::id() const
{
    if (id_.empty()) {
        auto *ptr = static_cast<const void *>(chunk_.data());
        id_ = fmt::format("{0:p}+{1:#04x}", ptr, chunk_.size());
    }

    return id_;
}

std::string In_memory_store::repr() const
{
    auto *ptr = static_cast<const void *>(chunk_.data());

    return fmt::format("<In_memory_store address={0:p} size={1:#04x} compression='{2}'>",
                       ptr,
                       chunk_.size(),
                       compression_);
}

}  // namespace abi_v1
}  // namespace mlio
