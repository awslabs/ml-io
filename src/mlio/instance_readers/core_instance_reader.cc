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

#include "mlio/instance_readers/core_instance_reader.h"

#include <exception>
#include <system_error>
#include <utility>

#include <fmt/format.h>

#include "mlio/data_reader.h"
#include "mlio/data_reader_error.h"
#include "mlio/instance.h"
#include "mlio/memory/memory_allocator.h"
#include "mlio/memory/memory_block.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/not_supported_error.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_error.h"
#include "mlio/streams/stream_error.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Core_instance_reader::Core_instance_reader(const Data_reader_params &params,
                                           Record_reader_factory &&factory)
    : params_{&params}, record_reader_factory_{std::move(factory)}
{
    store_iter_ = params_->dataset.begin();
}

std::optional<Instance> Core_instance_reader::read_instance_core()
{
    std::optional<Memory_slice> payload;
    try {
        payload = read_record_payload();
    }
    catch (const std::exception &) {
        handle_errors();
    }

    if (payload == std::nullopt) {
        // If we have not reached the end of the dataset, but we could
        // not read a record from the current data store, it means the
        // data store itself is an instance (e.g. an image file).
        if (store_ != nullptr) {
            return Instance{*store_};
        }

        return {};
    }

    return Instance{*store_, instance_idx_++, std::move(*payload)};
}

void Core_instance_reader::handle_errors()
{
    try {
        throw;
    }
    catch (const Record_too_large_error &) {
        std::throw_with_nested(Data_reader_error{fmt::format(
            "The record #{1:n} in the data store '{0}' is too large. See nested exception for details.",
            store_->id(),
            record_idx_)});
    }
    catch (const Corrupt_record_error &) {
        std::throw_with_nested(Data_reader_error{fmt::format(
            "The record #{1:n} in the data store '{0}' is corrupt. See nested exception for details.",
            store_->id(),
            record_idx_)});
    }
    catch (const Stream_error &) {
        std::throw_with_nested(Data_reader_error{fmt::format(
            "The data store '{0}' contains corrupt data. See nested exception for details.",
            store_->id())});
    }
    catch (const Not_supported_error &) {
        std::throw_with_nested(Data_reader_error{
            fmt::format("The data store '{0}' cannot be read. See nested exception for details.",
                        store_->id())});
    }
    catch (const std::system_error &) {
        std::throw_with_nested(Data_reader_error{fmt::format(
            "A system error occurred while trying to read from the data store '{0}'. See nested exception for details.",
            store_->id())});
    }
}

std::optional<Memory_slice> Core_instance_reader::read_record_payload()
{
    if (has_corrupt_split_record_) {
        throw_corrupt_split_record_error();
    }

    std::optional<Record> record = read_record();
    if (record == std::nullopt) {
        return {};
    }

    if (record->kind() == Record_kind::complete) {
        return std::move(*record).payload();
    }

    return read_split_record_payload(std::move(record));
}

std::optional<Memory_slice>
Core_instance_reader::read_split_record_payload(std::optional<Record> record)
{
    std::vector<Record> records{};

    std::size_t payload_size = 0;

    // A split record must start with a 'begin' record...
    if (record->kind() == Record_kind::begin) {
        payload_size += record->payload().size();

        records.emplace_back(std::move(*record));
    }
    else {
        throw_corrupt_split_record_error();
    }

    // continue with zero or more 'middle' records...
    while ((record = read_record()) && record->kind() == Record_kind::middle) {
        payload_size += record->payload().size();

        records.emplace_back(std::move(*record));
    }

    // and end with an 'end' record.
    if (record && record->kind() == Record_kind::end) {
        payload_size += record->payload().size();

        records.emplace_back(std::move(*record));
    }
    else {
        throw_corrupt_split_record_error();
    }

    // Once we have collected all records we merge their payloads in a
    // single buffer.
    auto payload = memory_allocator().allocate(payload_size);

    auto pos = payload->begin();
    for (const Record &r : records) {
        pos = std::copy(r.payload().begin(), r.payload().end(), pos);
    }

    return std::move(payload);
}

void Core_instance_reader::throw_corrupt_split_record_error()
{
    has_corrupt_split_record_ = true;

    throw Corrupt_record_error{"Corrupt split Record encountered."};
}

std::optional<Record> Core_instance_reader::read_record()
{
    if (record_reader_ == nullptr) {
        if (!init_next_record_reader()) {
            return {};
        }
    }

    std::optional<Record> record{};

    while ((record = record_reader_->read_record()) == std::nullopt) {
        if (!init_next_record_reader()) {
            return {};
        }
    }

    record_idx_++;

    return record;
}

bool Core_instance_reader::init_next_record_reader()
{
    instance_idx_ = 0;

    record_idx_ = 0;

    if (store_iter_ == params_->dataset.end()) {
        store_ = nullptr;

        record_reader_ = nullptr;

        return false;
    }

    store_ = store_iter_->get();

    try {
        record_reader_ = record_reader_factory_(*store_);
    }
    catch (const std::system_error &e) {
        if (e.code() == std::errc::no_such_file_or_directory) {
            throw Data_reader_error{
                fmt::format("The data store '{0}' does not exist.", store_->id())};
        }

        if (e.code() == std::errc::permission_denied) {
            throw Data_reader_error{fmt::format(
                "The permission to read the data store '{0}' is denied.", store_->id())};
        }

        throw;
    }

    // Move to the next data store only after we get the record reader;
    // otherwise we might break the class invariant if the factory
    // throws an exception.
    ++store_iter_;

    return record_reader_ != nullptr;
}

void Core_instance_reader::reset_core() noexcept
{
    store_iter_ = params_->dataset.begin();

    store_ = nullptr;

    record_reader_ = nullptr;

    instance_idx_ = 0;

    record_idx_ = 0;

    has_corrupt_split_record_ = false;
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
