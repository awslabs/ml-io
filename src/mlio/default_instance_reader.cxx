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

#include "mlio/default_instance_reader.h"

#include <algorithm>
#include <exception>
#include <iostream>
#include <system_error>
#include <type_traits>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>

#include "mlio/data_reader.h"
#include "mlio/data_reader_error.h"
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/not_supported_error.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_error.h"
#include "mlio/streams/stream_error.h"

namespace mlio {
inline namespace v1 {
namespace detail {

default_instance_reader::default_instance_reader(data_reader_params const &prm,
                                                 record_reader_factory &&fct)
    : params_{&prm}
    , record_reader_factory_{std::move(fct)}
    , num_shards_{std::max(params_->num_shards, 1UL)}
    , next_instance_idx_to_read_{params_->shard_index}
{
    if (params_->shard_index >= num_shards_) {
        throw std::invalid_argument{
            "The shard index must be less than the number of shards."};
    }

    store_iter_ = params_->dataset.begin();
}

std::optional<instance>
default_instance_reader::read_instance_core()
{
    if (should_stop_reading() || !skip_instances()) {
        return {};
    }

    num_instances_read_++;

    return read_instance_internal();
}

bool
default_instance_reader::should_stop_reading() const noexcept
{
    if (params_->num_instances_to_read != std::nullopt) {
        if (num_instances_read_ == *params_->num_instances_to_read) {
            return true;
        }
    }
    return false;
}

bool
default_instance_reader::skip_instances()
{
    while (num_instances_skipped_ < params_->num_instances_to_skip) {
        if (read_instance_internal() == std::nullopt) {
            return false;
        }

        num_instances_skipped_++;
    }

    return true;
}

std::optional<instance>
default_instance_reader::read_instance_internal()
{
    std::optional<memory_slice> payload;

    try {
        // In case sharding is enabled, we skip to the next element in
        // our shard.
        for (; instance_idx_ < next_instance_idx_to_read_; instance_idx_++) {
            if ((payload = read_record_payload()) == std::nullopt) {
                return {};
            }
        }

        if ((payload = read_record_payload())) {
            instance_idx_++;

            next_instance_idx_to_read_ += num_shards_;

            return instance{
                *store_, store_instance_idx_++, std::move(*payload)};
        }
    }
    catch (std::exception const &) {
        handle_nested_errors();
    }

    return {};
}

void
default_instance_reader::handle_nested_errors()
{
    try {
        throw;
    }
    catch (corrupt_record_error const &) {
        std::throw_with_nested(data_reader_error{
            fmt::format("The record {1:n} in the data store {0} is corrupt. "
                        "See nested exception for details.",
                        *store_,
                        store_record_idx_)});
    }
    catch (stream_error const &) {
        std::throw_with_nested(data_reader_error{
            fmt::format("The data store {0} contains corrupt data. "
                        "See nested exception for details.",
                        *store_)});
    }
    catch (not_supported_error const &) {
        std::throw_with_nested(
            data_reader_error{fmt::format("The data store {0} cannot be read. "
                                          "See nested exception for details.",
                                          *store_)});
    }
    catch (std::system_error const &) {
        std::throw_with_nested(data_reader_error{fmt::format(
            "A system error occurred while trying to read from the data "
            "store {0}. "
            "See nested exception for details.",
            *store_)});
    }
}

std::optional<memory_slice>
default_instance_reader::read_record_payload()
{
    std::optional<record> rec = read_record();
    if (rec == std::nullopt) {
        return {};
    }

    if (rec->kind() != record_kind::complete) {
        throw not_supported_error{"Split records are not supported yet!"};
    }

    num_bytes_read_ += rec->size();

    store_record_idx_++;

    return std::move(*rec).payload();
}

std::optional<record>
default_instance_reader::read_record()
{
    std::optional<record> rec{};

    while (record_reader_ == nullptr ||
           (rec = record_reader_->read_record()) == std::nullopt) {
        if (!init_next_record_reader()) {
            return {};
        }
    }

    return rec;
}

bool
default_instance_reader::init_next_record_reader()
{
    if (store_iter_ == params_->dataset.end()) {
        return false;
    }

    store_record_idx_ = 0;

    store_instance_idx_ = 0;

    store_ = store_iter_->get();

    try {
        record_reader_ = record_reader_factory_(*store_);
    }
    catch (std::system_error const &e) {
        if (e.code() == std::errc::no_such_file_or_directory) {
            throw data_reader_error{
                fmt::format("The data store {0} does not exist.", *store_)};
        }

        if (e.code() == std::errc::permission_denied) {
            throw data_reader_error{fmt::format(
                "The permission to read the data store {0} is denied.",
                *store_)};
        }

        throw;
    }

    // Move to the next data store after we get the reader instance;
    // otherwise we might break the class invariant if the factory
    // throws an exception.
    ++store_iter_;

    return true;
}

void
default_instance_reader::reset() noexcept
{
    store_iter_ = params_->dataset.begin();

    store_ = nullptr;

    record_reader_ = nullptr;

    num_bytes_read_ = 0;

    store_record_idx_ = 0;

    store_instance_idx_ = 0;

    instance_idx_ = 0;

    next_instance_idx_to_read_ = params_->shard_index;

    num_instances_skipped_ = 0;

    num_instances_read_ = 0;
}

}  // namespace detail
}  // namespace v1
}  // namespace mlio
