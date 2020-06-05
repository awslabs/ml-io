
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

#include "mlio/csv_reader.h"

#include <atomic>
#include <exception>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include <fmt/format.h>
#include <tbb/tbb.h>

#include "mlio/cpu_array.h"
#include "mlio/csv_record_tokenizer.h"
#include "mlio/data_reader.h"
#include "mlio/data_reader_error.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/example.h"
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/logger.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/csv_record_reader.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_error.h"
#include "mlio/record_readers/record_reader.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/utf8_input_stream.h"
#include "mlio/tensor.h"

using mlio::detail::Csv_record_reader;
using mlio::detail::Csv_record_tokenizer;

namespace mlio {
inline namespace abi_v1 {

struct Csv_reader::Decoder_state {
    explicit Decoder_state(const Csv_reader &r, std::vector<Intrusive_ptr<Tensor>> &t) noexcept;

    const Csv_reader *reader;
    std::vector<Intrusive_ptr<Tensor>> *tensors;
    bool warn_bad_instance;
    bool error_bad_example;
};

template<typename Col_iter>
class Csv_reader::Decoder {
public:
    explicit Decoder(Decoder_state &state,
                     Csv_record_tokenizer &tokenizer,
                     Col_iter col_beg,
                     Col_iter col_end)
        : state_{&state}, tokenizer_{&tokenizer}, col_beg_{col_beg}, col_end_{col_end}
    {}

    bool decode(std::size_t row_idx, const Instance &instance);

private:
    Decoder_state *state_;
    Csv_record_tokenizer *tokenizer_;
    Col_iter col_beg_;
    Col_iter col_end_;
};

Csv_reader::Csv_reader(Data_reader_params params, Csv_params csv_params)
    : Parallel_data_reader{std::move(params)}, params_{std::move(csv_params)}
{
    column_names_ = params_.column_names;
}

Csv_reader::~Csv_reader()
{
    stop();
}

void Csv_reader::reset() noexcept
{
    Parallel_data_reader::reset();

    should_read_header = true;
}

Intrusive_ptr<Record_reader> Csv_reader::make_record_reader(const Data_store &store)
{
    auto stream = make_utf8_stream(store.open_read(), params_.encoding);

    auto reader = make_intrusive<Csv_record_reader>(std::move(stream), params_);

    if (params_.header_row_index) {
        // Check if the caller did not explicitly specified the column
        // names and requested us to infer them from the header.
        if (column_names_.empty()) {
            read_names_from_header(store, *reader);
        }
        else if (should_read_header || !params_.has_single_header) {
            skip_to_header_row(*reader);

            // Discard the header row.
            reader->read_record();
        }

        should_read_header = false;
    }

    return std::move(reader);
}

void Csv_reader::read_names_from_header(const Data_store &store, Record_reader &reader)
{
    skip_to_header_row(reader);

    try {
        std::optional<Record> hdr = reader.read_record();
        if (hdr == std::nullopt) {
            return;
        }

        Csv_record_tokenizer tokenizer{params_, hdr->payload()};
        while (tokenizer.next()) {
            std::string name;
            if (params_.name_prefix.empty()) {
                name = tokenizer.value();
            }
            else {
                name = params_.name_prefix + tokenizer.value();
            }
            column_names_.emplace_back(std::move(name));
        }
    }
    catch (const Corrupt_record_error &) {
        std::throw_with_nested(Schema_error{fmt::format(
            "The header row of the data store '{0}' cannot be read. See nested exception for details.",
            store.id())});
    }

    // If the header row was blank, we treat it as a single column with
    // an blank name.
    if (column_names_.empty()) {
        column_names_.emplace_back(params_.name_prefix);
    }
}

void Csv_reader::skip_to_header_row(Record_reader &reader)
{
    std::size_t header_idx = *params_.header_row_index;
    for (std::size_t i = 0; i < header_idx; i++) {
        std::optional<Record> record = reader.read_record();
        if (record == std::nullopt) {
            return;
        }
    }
}

Intrusive_ptr<const Schema> Csv_reader::infer_schema(const std::optional<Instance> &instance)
{
    // If we don't have any data rows and if the store has no header or
    // explicit column names, we have no way to infer the schema.
    if (instance == std::nullopt && column_names_.empty()) {
        return {};
    }

    infer_column_types(instance);

    set_or_validate_column_names(instance);

    apply_column_type_overrides();

    return init_parsers_and_make_schema();
}

void Csv_reader::infer_column_types(const std::optional<Instance> &instance)
{
    // If we don't have any data rows, assume that all fields are of the
    // default data type or of type string.
    if (instance == std::nullopt) {
        column_types_.reserve(column_names_.size());

        for (std::size_t i = 0; i < column_names_.size(); i++) {
            Data_type dt{};
            if (params_.default_data_type == std::nullopt) {
                dt = Data_type::string;
            }
            else {
                dt = *params_.default_data_type;
            }
            column_types_.emplace_back(dt);
        }
    }
    else {
        try {
            Csv_record_tokenizer tokenizer{params_, instance->bits()};
            while (tokenizer.next()) {
                Data_type dt{};
                if (params_.default_data_type == std::nullopt) {
                    dt = infer_data_type(tokenizer.value());
                }
                else {
                    dt = *params_.default_data_type;
                }
                column_types_.emplace_back(dt);
            }
        }
        catch (const Corrupt_record_error &) {
            std::throw_with_nested(Schema_error{fmt::format(
                "The schema of the data store '{0}' cannot be inferred. See nested exception for details.",
                instance->data_store().id())});
        }
    }
}

void Csv_reader::set_or_validate_column_names(const std::optional<Instance> &instance)
{
    if (column_names_.empty()) {
        column_names_.reserve(column_types_.size());

        for (std::size_t idx = 1; idx <= column_types_.size(); idx++) {
            std::string name{};
            if (params_.name_prefix.empty()) {
                name = fmt::to_string(idx);
            }
            else {
                name = params_.name_prefix + fmt::to_string(idx);
            }
            column_names_.emplace_back(std::move(name));
        }
    }
    else {
        if (column_names_.size() != column_types_.size()) {
            throw Schema_error{fmt::format(
                "The number of columns ({3:n}) read from the row #{1:n} in the data store '{0}' does not match the number of headers ({2:n}).",
                instance->data_store().id(),
                instance->index(),
                column_names_.size(),
                column_types_.size())};
        }
    }
}

void Csv_reader::apply_column_type_overrides()
{
    std::size_t num_columns = column_names_.size();

    auto idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto idx_end = tbb::counting_iterator<std::size_t>(num_columns);

    auto name_beg = column_names_.begin();
    auto name_end = column_names_.end();

    auto type_beg = column_types_.begin();
    auto type_end = column_types_.end();

    auto col_beg = tbb::make_zip_iterator(idx_beg, name_beg, type_beg);
    auto col_end = tbb::make_zip_iterator(idx_end, name_end, type_end);

    // Override column types by index.
    auto idx_overrides = params_.column_types_by_index;

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        auto idx_type_pos = idx_overrides.find(std::get<0>(*col_pos));
        if (idx_type_pos != idx_overrides.end()) {
            std::get<2>(*col_pos) = idx_type_pos->second;

            idx_overrides.erase(idx_type_pos);
        }
    }

    // Throw an error if there are leftover indices.
    if (!idx_overrides.empty()) {
        std::vector<std::size_t> leftover_indices{};
        leftover_indices.reserve(idx_overrides.size());

        for (auto &pr : idx_overrides) {
            leftover_indices.emplace_back(pr.first);
        }

        throw std::invalid_argument{fmt::format(
            "The column types cannot be set. The following column indices are out of range: {0}",
            fmt::join(leftover_indices, ", "))};
    }

    // Override column types by name.
    auto name_overrides = params_.column_types;

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        auto name_type_pos = name_overrides.find(std::get<1>(*col_pos));
        if (name_type_pos != name_overrides.end()) {
            std::get<2>(*col_pos) = name_type_pos->second;

            name_overrides.erase(name_type_pos);
        }
    }

    // Throw an error if there are leftover names.
    if (!name_overrides.empty()) {
        std::vector<std::string> leftover_names{};
        leftover_names.reserve(name_overrides.size());

        for (auto &pr : name_overrides) {
            leftover_names.emplace_back(pr.first);
        }

        throw std::invalid_argument{fmt::format(
            "The column types cannot be set. The following columns are not found in the dataset: {0}",
            fmt::join(leftover_names, ", "))};
    }
}

Intrusive_ptr<const Schema> Csv_reader::init_parsers_and_make_schema()
{
    std::size_t batch_size = params().batch_size;

    std::vector<Attribute> attrs{};

    std::size_t num_columns = column_names_.size();

    column_ignores_.reserve(num_columns);
    column_parsers_.reserve(num_columns);

    auto idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto idx_end = tbb::counting_iterator<std::size_t>(num_columns);

    auto name_beg = column_names_.begin();
    auto name_end = column_names_.end();

    auto type_beg = column_types_.begin();
    auto type_end = column_types_.end();

    auto col_beg = tbb::make_zip_iterator(idx_beg, name_beg, type_beg);
    auto col_end = tbb::make_zip_iterator(idx_end, name_end, type_end);

    std::unordered_map<std::string, std::size_t> name_counts{};

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        std::string name = std::get<1>(*col_pos);

        if (should_skip(std::get<0>(*col_pos), name)) {
            column_ignores_.emplace_back(1);
            column_parsers_.emplace_back(nullptr);

            continue;
        }

        Data_type dt = std::get<2>(*col_pos);

        column_ignores_.emplace_back(0);
        column_parsers_.emplace_back(make_parser(dt, params_.parser_options));

        if (params_.dedupe_column_names) {
            // Keep count of column names. If the key already exists,
            // create a new name by appending an underscore plus count.
            // Since this new name might also exist, iterate until we
            // can insert the new name.
            auto [pos, inserted] = name_counts.try_emplace(name, 0);
            while (!inserted) {
                name.append("_").append(fmt::to_string(pos->second++));
                std::tie(pos, inserted) = name_counts.try_emplace(name, 0);
            }
            pos->second++;
        }

        attrs.emplace_back(std::move(name), dt, Size_vector{batch_size, 1});
    }

    try {
        return make_intrusive<Schema>(attrs);
    }
    catch (const std::invalid_argument &) {
        std::unordered_set<std::string_view> tmp{};
        for (auto &attr : attrs) {
            if (auto pr = tmp.emplace(attr.name()); !pr.second) {
                throw Schema_error{fmt::format(
                    "The dataset contains more than one column with the name '{0}'.", *pr.first)};
            }
        }

        throw;
    }
}

bool Csv_reader::should_skip(std::size_t index, const std::string &name) const noexcept
{
    auto uci = params_.use_columns_by_index;
    if (!uci.empty()) {
        if (uci.find(index) == uci.end()) {
            return true;
        }
    }

    auto ucn = params_.use_columns;
    if (!ucn.empty()) {
        if (ucn.find(name) == ucn.end()) {
            return true;
        }
    }

    return false;
}

Intrusive_ptr<Example> Csv_reader::decode(const Instance_batch &batch) const
{
    auto tensors = make_tensors(batch.size());

    Decoder_state state{*this, tensors};

    std::size_t num_instances = batch.instances().size();

    constexpr std::size_t cut_off = 10'000'000;

    bool should_run_serial =
        // If bad example handling mode is pad, we cannot parallelize
        // decoding as good records must be stacked together without
        // any gap in between.
        params().bad_example_handling == Bad_example_handling::pad ||
        params().bad_example_handling == Bad_example_handling::pad_warn ||
        // If the number of values (e.g. integers, floating-points) we
        // need to decode is below the cut-off threshold, avoid parallel
        // execution; otherwise the threading overhead will potentially
        // slow down the performance.
        column_names_.size() * num_instances < cut_off;

    std::optional<std::size_t> num_instances_read{};
    if (should_run_serial) {
        num_instances_read = decode_ser(state, batch);
    }
    else {
        num_instances_read = decode_prl(state, batch);
    }

    // Check if we failed to decode the example and return a null
    // pointer if that is the case.
    if (num_instances_read == std::nullopt) {
        if (params().bad_example_handling == Bad_example_handling::skip_warn) {
            logger::warn("The example #{0:n} has been skipped as it had at least one bad instance.",
                         batch.index());
        }

        return nullptr;
    }

    if (num_instances != *num_instances_read) {
        if (params().bad_example_handling == Bad_example_handling::pad_warn) {
            logger::warn("The example #{0:n} has been padded as it had {1:n} bad instance(s).",
                         batch.index(),
                         num_instances - *num_instances_read);
        }
    }

    auto example = make_intrusive<Example>(schema(), std::move(tensors));

    example->padding = batch.size() - *num_instances_read;

    return example;
}

std::vector<Intrusive_ptr<Tensor>> Csv_reader::make_tensors(std::size_t batch_size) const
{
    std::vector<Intrusive_ptr<Tensor>> tensors{};
    tensors.reserve(column_types_.size() - column_ignores_.size());

    auto type_beg = column_types_.begin();
    auto type_end = column_types_.end();

    auto ignore_beg = column_ignores_.begin();
    auto ignore_end = column_ignores_.end();

    auto col_beg = tbb::make_zip_iterator(type_beg, ignore_beg);
    auto col_end = tbb::make_zip_iterator(type_end, ignore_end);

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        // Check if we should skip this column.
        if (std::get<1>(*col_pos) != 0) {
            continue;
        }

        Size_vector shape{batch_size, 1};

        Data_type dt = std::get<0>(*col_pos);

        std::unique_ptr<Device_array> arr = make_cpu_array(dt, batch_size);

        tensors.emplace_back(make_intrusive<Dense_tensor>(std::move(shape), std::move(arr)));
    }

    return tensors;
}

auto Csv_reader::make_column_iterators() const noexcept
{
    std::size_t num_columns = column_names_.size();

    auto col_idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto col_idx_end = tbb::counting_iterator<std::size_t>(num_columns);

    auto name_beg = column_names_.begin();
    auto name_end = column_names_.end();

    auto type_beg = column_types_.begin();
    auto type_end = column_types_.end();

    auto ignore_beg = column_ignores_.begin();
    auto ignore_end = column_ignores_.end();

    auto parser_beg = column_parsers_.begin();
    auto parser_end = column_parsers_.end();

    auto col_beg = tbb::make_zip_iterator(col_idx_beg, name_beg, type_beg, ignore_beg, parser_beg);
    auto col_end = tbb::make_zip_iterator(col_idx_end, name_end, type_end, ignore_end, parser_end);

    return std::make_pair(col_beg, col_end);
}

std::optional<std::size_t>
Csv_reader::decode_ser(Decoder_state &state, const Instance_batch &batch) const
{
    std::size_t row_idx = 0;

    Csv_record_tokenizer tokenizer{params_};

    auto [col_beg, col_end] = make_column_iterators();

    for (const Instance &instance : batch.instances()) {
        Decoder<decltype(col_beg)> decoder{state, tokenizer, col_beg, col_end};
        if (decoder.decode(row_idx, instance)) {
            row_idx++;
        }
        else {
            // If the user requested to skip the example in case of an
            // error, shortcut the loop and return immediately.
            if (params().bad_example_handling == Bad_example_handling::skip ||
                params().bad_example_handling == Bad_example_handling::skip_warn) {
                return {};
            }
            if (params().bad_example_handling != Bad_example_handling::pad &&
                params().bad_example_handling != Bad_example_handling::pad_warn) {
                throw std::invalid_argument{"The specified bad example handling is invalid."};
            }
        }
    }

    return row_idx;
}

std::optional<std::size_t>
Csv_reader::decode_prl(Decoder_state &state, const Instance_batch &batch) const
{
    std::atomic_bool skip_example{};

    std::size_t num_instances = batch.instances().size();

    auto row_idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto row_idx_end = tbb::counting_iterator<std::size_t>(num_instances);

    auto instance_beg = batch.instances().begin();
    auto instance_end = batch.instances().end();

    auto range_beg = tbb::make_zip_iterator(row_idx_beg, instance_beg);
    auto range_end = tbb::make_zip_iterator(row_idx_end, instance_end);

    tbb::blocked_range<decltype(range_beg)> range{range_beg, range_end};

    auto worker = [this, &state, &skip_example](auto &sub_range) {
        Csv_record_tokenizer tokenizer{params_};

        // Both GCC and clang have trouble handling structured bindings
        // in lambdas.
        auto iter_pair = make_column_iterators();

        using Col_iter = std::remove_reference_t<decltype(std::get<0>(iter_pair))>;

        Col_iter col_beg = std::get<0>(iter_pair);
        Col_iter col_end = std::get<1>(iter_pair);

        for (auto instance_zip : sub_range) {
            // Both GCC and clang have a bug that prevents using class
            // template argument deduction (CTAD) with nested types.
            Decoder<Col_iter> decoder{state, tokenizer, col_beg, col_end};
            if (!decoder.decode(std::get<0>(instance_zip), std::get<1>(instance_zip))) {
                // If we failed to decode the instance, we can terminate
                // the task right away and skip this example.
                if (params().bad_example_handling == Bad_example_handling::skip ||
                    params().bad_example_handling == Bad_example_handling::skip_warn) {
                    skip_example = true;

                    return;
                }

                throw std::invalid_argument{"The specified bad example handling is invalid."};
            }
        }
    };

    tbb::parallel_for(range, worker, tbb::auto_partitioner{});

    if (skip_example) {
        return {};
    }

    return num_instances;
}

Csv_reader::Decoder_state::Decoder_state(const Csv_reader &r,
                                         std::vector<Intrusive_ptr<Tensor>> &t) noexcept
    : reader{&r}
    , tensors{&t}
    , warn_bad_instance{r.warn_bad_instances()}
    , error_bad_example{r.params().bad_example_handling == Bad_example_handling::error}
{}

template<typename Col_iter>
bool Csv_reader::Decoder<Col_iter>::decode(std::size_t row_idx, const Instance &instance)
{
    auto col_pos = col_beg_;

    auto tsr_pos = state_->tensors->begin();

    tokenizer_->reset(instance.bits());

    while (tokenizer_->next()) {
        if (col_pos == col_end_) {
            break;
        }

        // Check if we should skip this column.
        if (std::get<3>(*col_pos) != 0) {
            ++col_pos;

            continue;
        }

        // Check if we truncated the field.
        if (tokenizer_->truncated()) {
            auto h = state_->reader->params_.max_field_length_handling;

            if (h == Max_field_length_handling::treat_as_bad ||
                h == Max_field_length_handling::truncate_warn) {
                const std::string &name = std::get<1>(*col_pos);

                auto msg = fmt::format(
                    "The column '{2}' of the row #{1:n} in the data store '{0}' is too long. Its truncated value is '{3:.64}'.",
                    instance.data_store().id(),
                    instance.index(),
                    name,
                    tokenizer_->value());

                if (h == Max_field_length_handling::truncate_warn) {
                    logger::warn(msg);
                }
                else {
                    if (state_->warn_bad_instance || state_->error_bad_example) {
                        if (state_->warn_bad_instance) {
                            logger::warn(msg);
                        }

                        if (state_->error_bad_example) {
                            throw Invalid_instance_error{msg};
                        }
                    }

                    return false;
                }
            }
            else if (h != Max_field_length_handling::truncate) {
                throw std::invalid_argument{
                    "The specified maximum field length handling is invalid."};
            }
        }

        const Parser &parser = std::get<4>(*col_pos);

        auto &dense_tensor = static_cast<Dense_tensor &>(**tsr_pos);

        Parse_result r = parser(tokenizer_->value(), dense_tensor.data(), row_idx);
        if (r == Parse_result::ok) {
            ++col_pos;
            ++tsr_pos;

            continue;
        }

        if (state_->warn_bad_instance || state_->error_bad_example) {
            const std::string &name = std::get<1>(*col_pos);

            Data_type dt = std::get<2>(*col_pos);

            auto msg = fmt::format(
                "The column '{2}' of the row #{1:n} in the data store '{0}' cannot be parsed as {3}. Its string value is '{4:.64}'.",
                instance.data_store().id(),
                instance.index(),
                name,
                dt,
                tokenizer_->value());

            if (state_->warn_bad_instance) {
                logger::warn(msg);
            }

            if (state_->error_bad_example) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    // Make sure we read all columns and there are no remaining fields.
    if (col_pos == col_end_ && tokenizer_->eof()) {
        return true;
    }

    if (state_->warn_bad_instance || state_->error_bad_example) {
        std::size_t num_columns = state_->reader->column_names_.size();

        std::size_t num_actual_cols = std::get<0>(*col_pos);
        while (tokenizer_->next()) {
            num_actual_cols++;
        }
        if (col_pos == col_end_) {
            num_actual_cols++;
        }

        auto msg = fmt::format(
            "The row #{1:n} in the data store '{0}' has {2:n} column(s) while it is expected to have {3:n} column(s).",
            instance.data_store().id(),
            instance.index(),
            num_actual_cols,
            num_columns);

        if (state_->warn_bad_instance) {
            logger::warn(msg);
        }

        if (state_->error_bad_example) {
            throw Invalid_instance_error{msg};
        }
    }

    return false;
}

}  // namespace abi_v1
}  // namespace mlio
