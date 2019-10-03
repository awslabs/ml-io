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

#include "mlio/record_readers/parquet_record_reader.h"

#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/corrupt_record_error.h"
#include "mlio/record_readers/record.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace v1 {
namespace detail {

stdx::optional<record>
parquet_record_reader::
decode_record(memory_slice &chunk, bool ignore_leftover)
{
    if (chunk.empty()) {
        return {};
    }

    // See https://github.com/apache/parquet-format for the spec of the
    // Parquet file format.

    if (chunk.size() < magic_number_size_) {
        if (ignore_leftover) {
            return {};
        }

        throw corrupt_header_error{
            "The record does not start with the Parquet magic number."};
    }

    auto pos = chunk.begin();

    if (!is_magic_number(pos)) {
        throw corrupt_header_error{
            "The record does not start with the Parquet magic number."};
    }

    // The absolute minimum Parquet record can be 12 bytes. Two magic
    // numbers, one at the start and one at the end of the record, plus
    // the 4-byte metadata length field.
    if (chunk.size() < (2 * magic_number_size_) + sizeof(std::uint32_t)) {
        if (ignore_leftover) {
            return {};
        }

        throw corrupt_footer_error{
            "The record does not have a valid Parquet footer."};
    }

    // Advance by the size of the magic number and the metadata length field.
    pos += as_ssize(magic_number_size_ + sizeof(std::uint32_t));

    // As we do not know the size of the records we have to scan
    // through the chunk. The last 4 bytes of a Parquet footer should
    // contain the magic number "PAR1"; so a naive way would be to look
    // just for the magic numbers in the chunk. However this will likely
    // result in false positives as the character sequence "PAR1" can
    // can also be found in the record payload. We use a heuristic
    // approach to prevent this issue as much as we can. Besides the
    // magic number we also check if we can find the file metadata.
    for (; pos <= chunk.end() - as_ssize(magic_number_size_); ++pos) {
        if (is_magic_number(pos) && is_footer(chunk, pos)) {
            pos += magic_number_size_;

            auto payload = chunk.first(pos);

            chunk = chunk.subslice(pos);

            return record{std::move(payload)};
        }
    }

    if (ignore_leftover) {
        return {};
    }

    throw corrupt_footer_error{
        "The record does not have a valid Parquet footer."};
}

inline bool
parquet_record_reader::
is_magic_number(memory_block::iterator pos) noexcept
{
    return as<std::uint32_t>(pos) == 0x3152'4150;  // PAR1
}

bool
parquet_record_reader::
is_footer(memory_slice const &chunk, memory_block::iterator pos) noexcept
{
    auto metadata_end = pos - as_ssize(sizeof(std::uint32_t));

    // The 4 bytes right before the footer magic number contain
    // the size of the metadata.
    auto metadata_size = as<std::uint32_t>(metadata_end);

    // The absolute minimum metadata can be 9 bytes. The four required
    // fields -version, schema, num_rows, row_groups- all with a 1-byte
    // header and 1-byte value, plus the stop field.
    if (metadata_size < 9 * sizeof(std::uint8_t)) {
        return false;
    }

    // If the size of the metadata does not fit into the file, it is
    // clearly not the footer.
    auto blob_size = as_size(metadata_end - chunk.begin());
    if (magic_number_size_ + metadata_size > blob_size) {
        return false;
    }

    // The last byte of a Thrift Compact struct is called the stop
    // field and it must be always zero.
    auto stop_field = as<std::uint8_t>(metadata_end - sizeof(std::uint8_t));
    if (stop_field != 0) {
        return false;
    }

    // Instead of actually decoding the metadata we simply check if the
    // first byte represents a Thrift Compact field header. The size of
    // the metadata, the stop field, and this check together give us
    // enough confidence that we found a real file metadata.
    if (!is_file_metadata_begin(metadata_end - metadata_size)) {
        return false;
    }

    return true;
}

inline bool
parquet_record_reader::
is_file_metadata_begin(memory_block::iterator pos) noexcept
{
    auto thrift_field_header = as<std::uint8_t>(pos);

    // This is a heuristic approach to determine if we have a valid
    // metadata at the given position. The first byte of the metadata
    // should have one of the following values which correspond to the
    // encoded Thrift Compact headers of the metadata fields.
    return
        thrift_field_header == 0x15 ||  // version
        thrift_field_header == 0x29 ||  // schema
        thrift_field_header == 0x36 ||  // num_rows
        thrift_field_header == 0x49 ||  // row_groups
        thrift_field_header == 0x59 ||  // key_value_metadata
        thrift_field_header == 0x68 ||  // created_by
        thrift_field_header == 0x79;    // column_orders
}

template<typename T>
inline T
parquet_record_reader::
as(memory_block::iterator pos) noexcept
{
    return *reinterpret_cast<T const *>(&*pos);
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
