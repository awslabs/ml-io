
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

#include "mlio/csv_reader.h"

#include <atomic>
#include <exception>
#include <stdexcept>
#include <tuple>
#include <utility>

#include <fmt/format.h>
#include <fmt/ostream.h>
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

using mlio::detail::csv_record_reader;
using mlio::detail::csv_record_tokenizer;

namespace mlio {
inline namespace v1 {

struct csv_reader::decoder_state {
    csv_reader const *reader;
    std::vector<intrusive_ptr<tensor>> *tensors;
    bad_batch_handling bbh;
};

template<typename ColIt>
class csv_reader::decoder {
public:
    explicit decoder(decoder_state &state,
                     csv_record_tokenizer &tk,
                     ColIt col_beg,
                     ColIt col_end)
        : state_{&state}, tokenizer_{&tk}, col_beg_{col_beg}, col_end_{col_end}
    {}

public:
    bool
    decode(std::size_t row_idx, instance const &ins);

private:
    decoder_state *state_;
    csv_record_tokenizer *tokenizer_;
    ColIt col_beg_;
    ColIt col_end_;
};

csv_reader::csv_reader(data_reader_params rdr_prm, csv_params csv_prm)
    : parallel_data_reader{std::move(rdr_prm)}, params_{std::move(csv_prm)}
{
    column_names_ = params_.column_names;
}

csv_reader::~csv_reader()
{
    stop();
}

intrusive_ptr<record_reader>
csv_reader::make_record_reader(data_store const &ds)
{
    auto strm = make_utf8_stream(ds.open_read(), params_.encoding);

    auto rdr = make_intrusive<csv_record_reader>(std::move(strm), params_);

    if (params_.header_row_index) {
        // Check if the caller did not explicitly specified the column
        // names and requested us to infer them from the header.
        if (column_names_.empty()) {
            read_names_from_header(ds, *rdr);
        }
        else if (should_read_header || !params_.has_single_header) {
            skip_to_header_row(*rdr);

            // Discard the header row.
            rdr->read_record();
        }

        should_read_header = false;
    }

    return std::move(rdr);
}

void
csv_reader::read_names_from_header(data_store const &ds, record_reader &rdr)
{
    skip_to_header_row(rdr);

    try {
        std::optional<record> hdr = rdr.read_record();
        if (hdr == std::nullopt) {
            return;
        }

        csv_record_tokenizer tokenizer{params_, hdr->payload()};
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
    catch (corrupt_record_error const &) {
        std::throw_with_nested(schema_error{
            fmt::format("The header row of the data store {0} cannot be read. "
                        "See nested exception for details.",
                        ds)});
    }

    // If the header row was blank, we treat it as a single column with
    // an empty name.
    if (column_names_.empty()) {
        column_names_.emplace_back(params_.name_prefix);
    }
}

void
csv_reader::skip_to_header_row(record_reader &rdr)
{
    std::size_t header_idx = *params_.header_row_index;
    for (std::size_t i = 0; i < header_idx; i++) {
        std::optional<record> rec = rdr.read_record();
        if (rec == std::nullopt) {
            return;
        }
    }
}

intrusive_ptr<schema const>
csv_reader::infer_schema(std::optional<instance> const &ins)
{
    // If we don't have any data rows and if the store has no header or
    // no explicit column names, we have no way of inferring a schema.
    if (ins == std::nullopt && column_names_.empty()) {
        return {};
    }

    infer_column_types(ins);

    set_or_validate_column_names(ins);

    apply_column_type_overrides();

    return init_parsers_and_make_schema();
}

void
csv_reader::infer_column_types(std::optional<instance> const &ins)
{
    // If we don't have any data rows, assume that all fields are of
    // the default data type or of type string.
    if (ins == std::nullopt) {
        column_types_.reserve(column_names_.size());

        for (std::size_t i = 0; i < column_names_.size(); i++) {
            data_type dt{};
            if (params_.default_data_type == std::nullopt) {
                dt = data_type::string;
            }
            else {
                dt = *params_.default_data_type;
            }
            column_types_.emplace_back(dt);
        }
    }
    else {
        try {
            csv_record_tokenizer tokenizer{params_, ins->bits()};
            while (tokenizer.next()) {
                data_type dt{};
                if (params_.default_data_type == std::nullopt) {
                    dt = infer_data_type(tokenizer.value());
                }
                else {
                    dt = *params_.default_data_type;
                }
                column_types_.emplace_back(dt);
            }
        }
        catch (corrupt_record_error const &) {
            std::throw_with_nested(schema_error{fmt::format(
                "The schema of the data store {0} cannot be inferred. "
                "See nested exception for details.",
                ins->get_data_store())});
        }
    }
}

void
csv_reader::set_or_validate_column_names(std::optional<instance> const &ins)
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
            throw schema_error{fmt::format(
                "The number of columns ({3:n}) read from the row {1:n} in the "
                "data store {0} does not match the number of headers ({2:n}).",
                ins->get_data_store(),
                ins->index() + 1,
                column_names_.size(),
                column_types_.size())};
        }
    }
}

void
csv_reader::apply_column_type_overrides()
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
        std::vector<std::size_t> leftover_inds{};
        leftover_inds.reserve(idx_overrides.size());

        for (auto &pr : idx_overrides) {
            leftover_inds.emplace_back(pr.first);
        }

        throw std::invalid_argument{fmt::format(
            "The column types cannot bet set. The following column indices "
            "are out of range: {0}",
            fmt::join(leftover_inds, ", "))};
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
            "The column types cannot be set. The following columns are not "
            "found in the dataset: {0}",
            fmt::join(leftover_names, ", "))};
    }
}

intrusive_ptr<schema const>
csv_reader::init_parsers_and_make_schema()
{
    std::size_t batch_size = params().batch_size;

    std::vector<attribute> attrs{};

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

        data_type dt = std::get<2>(*col_pos);

        column_ignores_.emplace_back(0);
        column_parsers_.emplace_back(make_parser(dt, params_.parser_prm));

        if (params_.dedupe_column_names) {
            // Keep count of column names. If the key already exists, create a
            // new name by appending underscore + count. Since this new name
            // might also exist, iterate until we can insert the new name.
            auto [pos, inserted] = name_counts.try_emplace(name, 0);
            while (!inserted) {
                name.append("_").append(fmt::to_string(pos->second++));
                std::tie(pos, inserted) = name_counts.try_emplace(name, 0);
            }
            pos->second++;
        }

        attrs.emplace_back(std::move(name), dt, size_vector{batch_size, 1});
    }

    try {
        return make_intrusive<schema>(attrs);
    }
    catch (std::invalid_argument const &) {
        std::unordered_set<std::string_view> tmp{};
        for (auto &attr : attrs) {
            if (auto pr = tmp.emplace(attr.name()); !pr.second) {
                throw schema_error{
                    fmt::format("The dataset contains more than one column "
                                "with the name '{0}'.",
                                *pr.first)};
            }
        }

        throw;
    }
}

bool
csv_reader::should_skip(std::size_t index,
                        std::string const &name) const noexcept
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

intrusive_ptr<example>
csv_reader::decode(instance_batch const &batch) const
{
    auto tensors = make_tensors(batch.size());

    decoder_state dec_state{this, &tensors, effective_bad_batch_handling()};

    std::atomic_bool skip_batch{};

    std::size_t num_instances = batch.instances().size();

    auto row_idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto row_idx_end = tbb::counting_iterator<std::size_t>(num_instances);

    auto rec_beg = batch.instances().begin();
    auto rec_end = batch.instances().end();

    auto rng_beg = tbb::make_zip_iterator(row_idx_beg, rec_beg);
    auto rng_end = tbb::make_zip_iterator(row_idx_end, rec_end);

    tbb::blocked_range<decltype(rng_beg)> range{rng_beg, rng_end};

    auto worker = [this, &dec_state, &skip_batch](auto &sub_range) {
        csv_record_tokenizer tokenizer{params_};

        std::size_t num_columns = column_names_.size();

        auto col_idx_beg = tbb::counting_iterator<std::size_t>(0);
        auto col_idx_end = tbb::counting_iterator<std::size_t>(num_columns);

        auto name_beg = column_names_.begin();
        auto name_end = column_names_.end();

        auto type_beg = column_types_.begin();
        auto type_end = column_types_.end();

        auto skip_beg = column_ignores_.begin();
        auto skip_end = column_ignores_.end();

        auto prs_beg = column_parsers_.begin();
        auto prs_end = column_parsers_.end();

        auto col_beg = tbb::make_zip_iterator(
            col_idx_beg, name_beg, type_beg, skip_beg, prs_beg);
        auto col_end = tbb::make_zip_iterator(
            col_idx_end, name_end, type_end, skip_end, prs_end);

        for (auto row_zip : sub_range) {
            decoder<decltype(col_beg)> dc{
                dec_state, tokenizer, col_beg, col_end};

            if (!dc.decode(std::get<0>(row_zip), std::get<1>(row_zip))) {
                skip_batch = true;

                return;
            }
        }
    };

    constexpr std::size_t cut_off = 10'000'000;
    if (column_names_.size() * num_instances < cut_off) {
        worker(range);
    }
    else {
        tbb::parallel_for(range, worker, tbb::auto_partitioner{});
    }

    if (skip_batch) {
        return nullptr;
    }

    auto exm = make_intrusive<example>(get_schema(), std::move(tensors));

    exm->padding = batch.size() - num_instances;

    return exm;
}

std::vector<intrusive_ptr<tensor>>
csv_reader::make_tensors(std::size_t batch_size) const
{
    std::vector<intrusive_ptr<tensor>> tensors{};
    tensors.reserve(column_types_.size() - column_ignores_.size());

    auto type_beg = column_types_.begin();
    auto type_end = column_types_.end();

    auto skip_beg = column_ignores_.begin();
    auto skip_end = column_ignores_.end();

    auto col_beg = tbb::make_zip_iterator(type_beg, skip_beg);
    auto col_end = tbb::make_zip_iterator(type_end, skip_end);

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        // Check if we should skip this column.
        if (std::get<1>(*col_pos) != 0) {
            continue;
        }

        data_type dt = std::get<0>(*col_pos);

        std::unique_ptr<device_array> arr = make_cpu_array(dt, batch_size);

        auto tsr = make_intrusive<dense_tensor>(size_vector{batch_size, 1},
                                                std::move(arr));

        tensors.emplace_back(std::move(tsr));
    }

    return tensors;
}

template<typename ColIt>
bool
csv_reader::decoder<ColIt>::decode(std::size_t row_idx, instance const &ins)
{
    auto col_pos = col_beg_;

    auto tsr_pos = state_->tensors->begin();

    tokenizer_->reset(ins.bits());

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
        if (tokenizer_->is_truncated()) {
            auto mflh = state_->reader->params_.max_field_length_hnd;

            if (mflh != max_field_length_handling::truncate) {
                std::string const &name = std::get<1>(*col_pos);

                auto msg = fmt::format(
                    "The column '{2}' of the row {1:n} in the data store {0} "
                    "was truncated. Its truncated string value is '{3:.64}'.",
                    ins.get_data_store(),
                    ins.index() + 1,
                    name,
                    tokenizer_->value());

                if (mflh == max_field_length_handling::error) {
                    throw field_too_large_error{msg};
                }

                logger::warn(msg);
            }
        }

        parser const &prsr = std::get<4>(*col_pos);

        auto &dense_tsr = static_cast<dense_tensor &>(**tsr_pos);

        parse_result r = prsr(tokenizer_->value(), dense_tsr.data(), row_idx);
        if (r == parse_result::ok) {
            ++col_pos;
            ++tsr_pos;

            continue;
        }

        if (state_->bbh != bad_batch_handling::skip) {
            std::string const &name = std::get<1>(*col_pos);

            data_type dt = std::get<2>(*col_pos);

            auto msg = fmt::format(
                "The column '{2}' of the row {1:n} in the data store {0} "
                "cannot be parsed as {3}. Its string value is '{4:.64}'.",
                ins.get_data_store(),
                ins.index() + 1,
                name,
                dt,
                tokenizer_->value());

            if (state_->bbh == bad_batch_handling::error) {
                throw invalid_instance_error{msg};
            }

            logger::warn(msg);
        }

        return false;
    }

    // Make sure we read all columns and there are no remaining fields.
    if (col_pos == col_end_ && tokenizer_->eof()) {
        return true;
    }

    if (state_->bbh != bad_batch_handling::skip) {
        std::size_t num_columns = state_->reader->column_names_.size();

        std::size_t num_actual_cols = std::get<0>(*col_pos);
        while (tokenizer_->next()) {
            num_actual_cols++;
        }
        if (col_pos == col_end_) {
            num_actual_cols++;
        }

        auto msg = fmt::format(
            "The row {1:n} in the data store {0} has {2:n} column(s), while "
            "the expected number of column(s) is {3:n}.",
            ins.get_data_store(),
            ins.index() + 1,
            num_actual_cols,
            num_columns);

        if (state_->bbh == bad_batch_handling::error) {
            throw invalid_instance_error{msg};
        }

        logger::warn(msg);
    }

    return false;
}

void
csv_reader::reset() noexcept
{
    parallel_data_reader::reset();

    should_read_header = true;
}

field_too_large_error::~field_too_large_error() = default;

}  // namespace v1
}  // namespace mlio
