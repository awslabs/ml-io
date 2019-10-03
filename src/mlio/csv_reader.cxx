
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
#include "mlio/data_reader_error.h"
#include "mlio/data_reader.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/example.h"
#include "mlio/instance_batch.h"
#include "mlio/instance.h"
#include "mlio/logger.h"
#include "mlio/memory/memory_slice.h"
#include "mlio/record_readers/corrupt_record_error.h"
#include "mlio/record_readers/csv_record_reader.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_reader.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream.h"
#include "mlio/streams/utf8_input_stream.h"
#include "mlio/tensor.h"

using mlio::detail::csv_record_reader;
using mlio::detail::csv_record_tokenizer;

namespace mlio {
inline namespace v1 {

csv_reader::
csv_reader(data_reader_params rdr_prm, csv_params csv_prm)
    : parallel_data_reader{std::move(rdr_prm)}, params_{std::move(csv_prm)}
{
    column_names_ = params_.column_names;
}

csv_reader::~csv_reader()
{
    stop();
}

intrusive_ptr<record_reader>
csv_reader::
make_record_reader(data_store const &ds)
{
    auto strm = make_utf8_stream(ds.open_read(), params_.encoding);

    auto rdr = make_intrusive<csv_record_reader>(std::move(strm), params_);

    // Check if the caller did not explicitly specified the column
    // names and requested us to infer them from the header.
    if (column_names_.empty()) {
        if (params_.header_row_index) {
            read_names_from_header(ds, *rdr);
        }
    } else {
        if (params_.header_row_index) {
            skip_to_header_row(*rdr);

            rdr->read_record();
        }
    }

    return std::move(rdr);
}

void
csv_reader::
read_names_from_header(data_store const &ds, record_reader &rdr)
{
    skip_to_header_row(rdr);

    try {
        stdx::optional<record> hdr = rdr.read_record();
        if (hdr == stdx::nullopt) {
            return;
        }

        csv_record_tokenizer tk{hdr->payload(), params_.delimiter};
        while (tk.next()) {
            std::string name;
            if (params_.name_prefix.empty()) {
                name = tk.value();
            } else {
                name = params_.name_prefix + tk.value();
            }
            column_names_.emplace_back(std::move(name));
        }
    }
    catch (corrupt_record_error const &) {
        std::throw_with_nested(
            schema_error{fmt::format(
                "The header row of the data store {0} cannot be read. "
                "See nested exception for details.", ds)});
    }

    // If the header row was blank, we treat it as a single
    // column with an empty name.
    if (column_names_.empty()) {
        column_names_.emplace_back(params_.name_prefix);
    }
}

void
csv_reader::
skip_to_header_row(record_reader &rdr)
{
    std::size_t header_idx = *params_.header_row_index;
    for (std::size_t i = 0; i < header_idx; i++) {
        stdx::optional<record> rec = rdr.read_record();
        if (rec == stdx::nullopt) {
            return;
        }
    }
}

void
csv_reader::
infer_schema(instance const &ins)
{
    infer_column_types(ins);

    set_or_validate_names(ins);

    apply_column_type_overrides();

    init_parsers_and_schema();
}

void
csv_reader::
infer_column_types(instance const &ins)
{
    try {
        csv_record_tokenizer tk{ins.bits(), params_.delimiter};
        while (tk.next()) {
            data_type dt;
            if (params_.default_data_type == stdx::nullopt) {
                dt = infer_data_type(tk.value());
            } else {
                dt = *params_.default_data_type;
            }
            column_types_.emplace_back(dt);
        }
    }
    catch (corrupt_record_error const &) {
        std::throw_with_nested(
            schema_error{fmt::format(
                "The schema of the data store {0} cannot be inferred. "
                "See nested exception for details.", ins.get_data_store())});
    }
}

void
csv_reader::
set_or_validate_names(instance const &ins)
{
    if (column_names_.empty()) {
        for (std::size_t idx = 1; idx <= column_types_.size(); idx++) {
            std::string name;
            if (params_.name_prefix.empty()) {
                name = fmt::to_string(idx);
            } else {
                name = params_.name_prefix + fmt::to_string(idx);
            }
            column_names_.emplace_back(std::move(name));
        }
    } else {
        if (column_names_.size() != column_types_.size()) {
            throw schema_error{fmt::format(
                "The number of columns ({3:n}) read from the row {1:n} in the "
                "data store {0} does not match the number of headers ({2:n}).",
                ins.get_data_store(), ins.index() + 1,
                column_names_.size(), column_types_.size())};

        }
    }
}

void
csv_reader::
apply_column_type_overrides()
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

    auto idx_overrides = params_.column_types_by_index;

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        auto idx_type_pos = idx_overrides.find(std::get<0>(*col_pos));
        if (idx_type_pos != idx_overrides.end()) {
            std::get<2>(*col_pos) = idx_type_pos->second;

            idx_overrides.erase(idx_type_pos);
        }
    }

    if (!idx_overrides.empty()) {
        std::vector<std::size_t> extra_inds;
        extra_inds.reserve(idx_overrides.size());

        for (auto &idx_type : idx_overrides) {
            extra_inds.emplace_back(idx_type.first);
        }

        throw std::invalid_argument{fmt::format(
            "The column types cannot bet set. The following column indices "
            "are out of range: {0}", fmt::join(extra_inds, ", "))};
    }

    auto name_overrides = params_.column_types;

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        auto name_type_pos = name_overrides.find(std::get<1>(*col_pos));
        if (name_type_pos != name_overrides.end()) {
            std::get<2>(*col_pos) = name_type_pos->second;

            name_overrides.erase(name_type_pos);
        }
    }

    if (!name_overrides.empty()) {
        std::vector<std::string> extra_names;
        extra_names.reserve(name_overrides.size());

        for (auto &name_type : name_overrides) {
            extra_names.emplace_back(name_type.first);
        }

        throw std::invalid_argument{fmt::format(
            "The column types cannot be set. The following columns are not "
            "found in the dataset: {0}", fmt::join(extra_names, ", "))};
    }
}

void
csv_reader::
init_parsers_and_schema()
{
    std::size_t batch_size = params().batch_size;

    std::vector<feature_desc> descs{};

    std::size_t num_columns = column_names_.size();

    auto idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto idx_end = tbb::counting_iterator<std::size_t>(num_columns);

    auto name_beg = column_names_.begin();
    auto name_end = column_names_.end();

    auto type_beg = column_types_.begin();
    auto type_end = column_types_.end();

    auto col_beg = tbb::make_zip_iterator(idx_beg, name_beg, type_beg);
    auto col_end = tbb::make_zip_iterator(idx_end, name_end, type_end);

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        std::size_t idx = std::get<0>(*col_pos);

        std::string const &name = std::get<1>(*col_pos);

        data_type dt = std::get<2>(*col_pos);

        if (should_skip(idx, name)) {
            column_skips_.emplace_back(1);

            column_parsers_.emplace_back(nullptr);
        } else {
            column_skips_.emplace_back(0);

            column_parsers_.emplace_back(make_parser(dt, params_.parser_prm));

            descs.emplace_back(
                feature_desc_builder{name, dt, {batch_size}}.build());
        }
    }

    schema_ = make_intrusive<schema>(std::move(descs));
}

bool
csv_reader::
should_skip(std::size_t index, std::string const &name) const noexcept
{
    auto use_cols_idx = params_.use_columns_by_index;
    if (!use_cols_idx.empty()) {
        if (use_cols_idx.find(index) == use_cols_idx.end()) {
            return true;
        }
    }

    auto use_cols_name = params_.use_columns;
    if (!use_cols_name.empty()) {
        if (use_cols_name.find(name) == use_cols_name.end()) {
            return true;
        }
    }

    return false;
}

intrusive_ptr<example>
csv_reader::
decode(instance_batch const &batch) const
{
    auto tensors = make_tensors(batch.size());

    auto bbh = effective_bad_batch_handling();

    std::atomic_bool skip_batch{};

    std::size_t num_instances = batch.instances().size();

    auto row_idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto row_idx_end = tbb::counting_iterator<std::size_t>(num_instances);

    auto rec_beg = batch.instances().begin();
    auto rec_end = batch.instances().end();

    auto rng_beg = tbb::make_zip_iterator(row_idx_beg, rec_beg);
    auto rng_end = tbb::make_zip_iterator(row_idx_end, rec_end);

    tbb::blocked_range<decltype(rng_beg)> range{rng_beg, rng_end};

    auto worker = [this, &tensors, bbh, &skip_batch](auto &sub_range)
    {
        for (auto row_zip : sub_range) {
            std::size_t row_idx = std::get<0>(row_zip);

            instance const &ins = std::get<1>(row_zip);

            std::size_t num_columns = column_names_.size();

            auto col_idx_beg = tbb::counting_iterator<std::size_t>(0);
            auto col_idx_end = tbb::counting_iterator<std::size_t>(num_columns);

            auto name_beg = column_names_.begin();
            auto name_end = column_names_.end();

            auto type_beg = column_types_.begin();
            auto type_end = column_types_.end();

            auto skip_beg = column_skips_.begin();
            auto skip_end = column_skips_.end();

            auto prs_beg = column_parsers_.begin();
            auto prs_end = column_parsers_.end();

            auto col_beg = tbb::make_zip_iterator(col_idx_beg, name_beg,
                                                  type_beg, skip_beg, prs_beg);
            auto col_end = tbb::make_zip_iterator(col_idx_end, name_end,
                                                  type_end, skip_end, prs_end);

            auto col_pos = col_beg;

            auto tsr_pos = tensors.begin();

            csv_record_tokenizer tk{ins.bits(), params_.delimiter};
            while (tk.next()) {
                // Check if we should skip this column.
                if (std::get<3>(*col_pos) != 0) {
                    ++col_pos;

                    continue;
                }

                if (col_pos == col_end) {
                    break;
                }

                parser const &prsr = std::get<4>(*col_pos);

                intrusive_ptr<tensor> const &tsr = *tsr_pos;

                auto &dense_tsr = static_cast<dense_tensor &>(*tsr);

                parse_result r = prsr(tk.value(), dense_tsr.data(), row_idx);
                if (r == parse_result::ok) {
                    ++col_pos;
                    ++tsr_pos;

                    continue;
                }

                if (bbh != bad_batch_handling::skip) {
                    std::string const &name = std::get<1>(*col_pos);

                    data_type dt = std::get<2>(*col_pos);

                    auto msg = fmt::format(
                        "The column '{2}' of the row {1:n} in the data store "
                        "{0} cannot be parsed as {3}. Its string value is "
                        "'{4:.64}'.",
                        ins.get_data_store(), ins.index() + 1, name, dt,
                        tk.value());

                    if (bbh == bad_batch_handling::error) {
                        throw invalid_instance_error{msg};
                    }

                    logger::warn(msg);
                }

                skip_batch = true;

                return;
            }

            // Make sure we read all columns and there are no remaining
            // fields.
            if (col_pos == col_end && tk.eof()) {
                continue;
            }

            if (bbh != bad_batch_handling::skip) {
                std::size_t num_actual_cols = std::get<0>(*col_pos);
                while (tk.next()) {
                    num_actual_cols++;
                }
                if (col_pos == col_end) {
                    num_actual_cols++;
                }

                auto msg = fmt::format(
                    "The row {1:n} in the data store {0} has {2:n} column(s), "
                    "while the expected number of column(s) is {3:n}.",
                    ins.get_data_store(), ins.index() + 1, num_actual_cols,
                    num_columns);

                if (bbh == bad_batch_handling::error) {
                    throw invalid_instance_error{msg};
                }

                logger::warn(msg);
            }

            skip_batch = true;

            return;
        }
    };

    constexpr std::size_t cut_off = 10'000'000;
    if (column_names_.size() * num_instances < cut_off) {
        worker(range);
    } else {
        tbb::parallel_for(range, worker, tbb::auto_partitioner{});
    }

    if (skip_batch) {
        return nullptr;
    }

    auto exm = make_intrusive<example>(schema_, std::move(tensors));

    exm->padding = batch.size() - num_instances;

    return exm;
}

std::vector<intrusive_ptr<tensor>>
csv_reader::
make_tensors(std::size_t batch_size) const
{
    std::vector<intrusive_ptr<tensor>> tensors;
    tensors.reserve(column_types_.size());

    auto type_beg = column_types_.begin();
    auto type_end = column_types_.end();

    auto skip_beg = column_skips_.begin();
    auto skip_end = column_skips_.end();

    auto col_beg = tbb::make_zip_iterator(type_beg, skip_beg);
    auto col_end = tbb::make_zip_iterator(type_end, skip_end);

    for (auto col_pos = col_beg; col_pos < col_end; ++col_pos) {
        if (std::get<1>(*col_pos) != 0) {
            continue;
        }

        data_type dt = std::get<0>(*col_pos);

        auto shp = {batch_size};

        auto arr = make_cpu_array(dt, batch_size);

        auto tsr = make_intrusive<dense_tensor>(shp, std::move(arr));

        tensors.emplace_back(std::move(tsr));
    }

    return tensors;
}

}  // namespace v1
}  // namespace mlio
