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

#include "mlio/instance.h"

#include <exception>
#include <system_error>

#include <fmt/format.h>

#include "mlio/data_reader_error.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_allocator.h"
#include "mlio/memory/memory_block.h"
#include "mlio/memory/util.h"
#include "mlio/not_supported_error.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/stream_error.h"

namespace mlio {
inline namespace v1 {

memory_slice instance::load_bits_from_store() const
{
    intrusive_ptr<input_stream> strm{};
    try {
        strm = store_->open_read();
    }
    catch (const std::system_error &e) {
        if (e.code() == std::errc::no_such_file_or_directory) {
            throw data_reader_error{
                fmt::format("The data store '{0}' does not exist.", store_->id())};
        }

        if (e.code() == std::errc::permission_denied) {
            throw data_reader_error{fmt::format(
                "The permission to read the data store '{0}' is denied.", store_->id())};
        }

        throw;
    }

    if (strm->supports_zero_copy()) {
        memory_slice bits{};
        try {
            bits = strm->read(strm->size());
        }
        catch (const std::exception &) {
            handle_errors();
        }

        // Although this shouldn't happen, make sure we have read all
        // the data.
        if (bits.size() == strm->size()) {
            return bits;
        }

        // Otherwise treat the stream as seekable.
        strm->seek(0);
    }

    return read_stream(*strm);
}

memory_slice instance::read_stream(input_stream &strm) const
{
    intrusive_ptr<mutable_memory_block> blk{};

    mutable_memory_span remaining_bits{};

    bool is_eof = false;

    while (!is_eof) {
        std::size_t prev_size{};
        if (blk) {
            prev_size = blk->size();

            blk = resize_memory_block(blk, prev_size * 2);
        }
        else {
            if (strm.seekable()) {
                blk = get_memory_allocator().allocate(strm.size());
            }
            else {
                blk = get_memory_allocator().allocate(0x100000);  // 1MiB
            }
        }

        remaining_bits = mutable_memory_span{*blk}.subspan(prev_size);

        while (!remaining_bits.empty()) {
            std::size_t num_bytes_read{};
            try {
                num_bytes_read = strm.read(remaining_bits);
            }
            catch (const std::exception &) {
                handle_errors();
            }

            if (num_bytes_read == 0) {
                is_eof = true;

                break;
            }

            remaining_bits = remaining_bits.subspan(num_bytes_read);
        }
    }

    return memory_slice{blk}.first(blk->size() - remaining_bits.size());
}

void instance::handle_errors() const
{
    try {
        throw;
    }
    catch (const stream_error &) {
        std::throw_with_nested(data_reader_error{fmt::format(
            "The data store '{0}' contains corrupt data. See nested exception for details.",
            store_->id())});
    }
    catch (const not_supported_error &) {
        std::throw_with_nested(data_reader_error{
            fmt::format("The data store '{0}' cannot be read. See nested exception for details.",
                        store_->id())});
    }
    catch (const std::system_error &) {
        std::throw_with_nested(data_reader_error{fmt::format(
            "A system error occurred while trying to read from the data store '{0}'. See nested exception for details.",
            store_->id())});
    }
}

}  // namespace v1
}  // namespace mlio
