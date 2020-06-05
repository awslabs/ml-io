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

#pragma once

#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_reader_error.h"
#include "mlio/data_type.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/parser.h"
#include "mlio/text_encoding.h"

namespace mlio {
inline namespace abi_v1 {

/// Specifies how field limits should be enforced.
enum class Max_field_length_handling {
    treat_as_bad,  ///< Treat the corresponding row as bad.
    truncate,      ///< Truncate the field.
    truncate_warn  ///< Truncate the field and warn.
};

/// @addtogroup data_readers Data Readers
/// @{

/// Holds the parameters for @ref Csv_reader.
struct MLIO_API Csv_params final {
    /// The index of the row that should be treated as the header of the
    /// dataset. If @ref column_names is empty, the column names will be
    /// inferred from that row.  If neither @ref header_row_index nor
    /// @ref column_names is specified, the column ordinal positions
    /// will be used as column names.
    ///
    /// @note
    ///     Each data store in the dataset should have its header at the
    ///     same index.
    std::optional<std::size_t> header_row_index = 0;
    /// A boolean value indicating whether the dataset has a header row
    /// only in the first data store.
    bool has_single_header = false;
    /// A boolean value indicating whether duplicate columns should be
    /// renamed. If true, duplicate columns 'X', ..., 'X' will be
    /// renamed to 'X', 'X_1', X_2', ...
    bool dedupe_column_names = true;
    /// The column names.
    ///
    /// @note
    ///     If the dataset has a header and @ref header_row_index is
    ///     specified, this list can be left empty to infer the column
    ///     names from the dataset.
    std::vector<std::string> column_names{};
    /// The prefix to prepend to column names.
    std::string name_prefix{};
    /// The columns that should be read. The rest of the columns will be
    /// skipped.
    std::unordered_set<std::string> use_columns{};
    /// The columns, specified by index, that should be read. The rest
    /// of the columns will be skipped.
    std::unordered_set<std::size_t> use_columns_by_index{};
    /// The data type for columns for which no explicit data type is
    /// specified via @ref column_types or @ref column_types_by_index.
    /// If not specified, the column data types will be inferred from
    /// the dataset.
    std::optional<Data_type> default_data_type{};
    /// The mapping between columns and data types by name.
    std::unordered_map<std::string, Data_type> column_types{};
    /// The mapping between columns and data types by index.
    std::unordered_map<std::size_t, Data_type> column_types_by_index{};
    /// The delimiter character.
    char delimiter = ',';
    /// The character used for quoting field values.
    char quote_char = '"';
    /// The comment character. Lines that start with the comment
    /// character will be skipped.
    std::optional<char> comment_char = std::nullopt;
    /// A boolean value indicating whether quoted fields can be multi-
    /// line. Note that turning this flag on can slow down the reading
    /// speed.
    bool allow_quoted_new_lines = false;
    /// A boolean value indicating whether to skip empty lines.
    bool skip_blank_lines = true;
    /// The text encoding to use for reading. If not specified, it will
    /// be inferred from the preamble of the text; otherwise falls back
    /// to UTF-8.
    std::optional<Text_encoding> encoding{};
    /// The maximum number of characters that will be read in a field.
    /// Any characters beyond this limit will be handled using the
    /// strategy in @ref max_field_length_handling.
    std::optional<std::size_t> max_field_length{};
    /// See @ref Max_field_length_handling.
    Max_field_length_handling max_field_length_handling = Max_field_length_handling::treat_as_bad;
    /// The maximum size of a text line. If a row is longer than the
    /// specified size, an error will be raised.
    std::optional<std::size_t> max_line_length{};
    /// Additional options relevant for field parsing.
    Parser_options parser_options{};
};

/// Represents a @ref Data_reader for reading CSV datasets.
class MLIO_API Csv_reader final : public Parallel_data_reader {
public:
    explicit Csv_reader(Data_reader_params params, Csv_params csv_params = {});

    Csv_reader(const Csv_reader &) = delete;

    Csv_reader &operator=(const Csv_reader &) = delete;

    Csv_reader(Csv_reader &&) = delete;

    Csv_reader &operator=(Csv_reader &&) = delete;

    ~Csv_reader() final;

    void reset() noexcept final;

private:
    struct Decoder_state;

    template<typename Col_iter>
    class Decoder;

    MLIO_HIDDEN
    Intrusive_ptr<Record_reader> make_record_reader(const Data_store &store) final;

    MLIO_HIDDEN
    void read_names_from_header(const Data_store &store, Record_reader &reader);

    MLIO_HIDDEN
    void skip_to_header_row(Record_reader &reader);

    MLIO_HIDDEN
    Intrusive_ptr<const Schema> infer_schema(const std::optional<Instance> &instance) final;

    MLIO_HIDDEN
    void infer_column_types(const std::optional<Instance> &instance);

    MLIO_HIDDEN
    void set_or_validate_column_names(const std::optional<Instance> &instance);

    MLIO_HIDDEN
    void apply_column_type_overrides();

    MLIO_HIDDEN
    Intrusive_ptr<const Schema> init_parsers_and_make_schema();

    MLIO_HIDDEN
    bool should_skip(std::size_t index, const std::string &name) const noexcept;

    MLIO_HIDDEN
    Intrusive_ptr<Example> decode(const Instance_batch &batch) const final;

    MLIO_HIDDEN
    std::vector<Intrusive_ptr<Tensor>> make_tensors(std::size_t batch_size) const;

    MLIO_HIDDEN
    std::optional<std::size_t> decode_ser(Decoder_state &state, const Instance_batch &batch) const;

    MLIO_HIDDEN
    std::optional<std::size_t> decode_prl(Decoder_state &state, const Instance_batch &batch) const;

    MLIO_HIDDEN
    auto make_column_iterators() const noexcept;

private:
    Csv_params params_;
    std::vector<std::string> column_names_;
    std::vector<Data_type> column_types_{};
    std::vector<int> column_ignores_{};
    std::vector<Parser> column_parsers_{};
    bool should_read_header = true;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
