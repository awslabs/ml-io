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

#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/optional.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/parser.h"
#include "mlio/schema.h"
#include "mlio/text_encoding.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Holds the parameters for @ref csv_reader.
struct MLIO_API csv_params final {
    /// The index of the row that should be treated as the header of the
    /// dataset. If specified, the column names will be inferred from
    /// that row; otherwise @ref column_names will be used. If neither
    /// @ref header_row_index nor @ref column_names is specified, the
    /// column ordinal positions will be used as column names
    ///
    /// @note
    ///     Each data store in the dataset should have its header at the
    ///     same index.
    stdx::optional<std::size_t> header_row_index = 0;
    /// The column names; ignored if @ref header_row_index is specified.
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
    stdx::optional<data_type> default_data_type{};
    /// The mapping between columns and data types by name.
    std::unordered_map<std::string, data_type> column_types{};
    /// The mapping between columns and data types by index.
    std::unordered_map<std::size_t, data_type> column_types_by_index{};
    /// The delimiter character.
    char delimiter = ',';
    /// The comment character. Lines that start with the comment
    /// character will be skipped.
    stdx::optional<char> comment_char = '#';
    /// A boolean value indicating whether quoted fields can be multi-
    /// line. Note that turning this flag on can slow down the reading
    /// speed.
    bool allow_quoted_new_lines = false;
    /// A boolean value indicating whether to skip empty lines.
    bool skip_blank_lines = true;
    /// The text encoding to use for reading. If not specified, it will
    /// be inferred from the preamble of the text; otherwise falls back
    /// to UTF-8.
    stdx::optional<text_encoding> encoding{};
    /// Additional parameters relevant for field parsing.
    parser_params parser_prm{};
};

/// Represents a @ref data_reader for reading CSV datasets.
class MLIO_API csv_reader final : public parallel_data_reader {
public:
    explicit
    csv_reader(data_reader_params rdr_prm, csv_params csv_prm);

    csv_reader(csv_reader const &) = delete;

    csv_reader(csv_reader &&) = delete;

   ~csv_reader() final;

public:
    csv_reader &
    operator=(csv_reader const &) = delete;

    csv_reader &
    operator=(csv_reader &&) = delete;

private:
    MLIO_HIDDEN
    intrusive_ptr<record_reader>
    make_record_reader(data_store const &ds) final;

    MLIO_HIDDEN
    void
    read_names_from_header(data_store const &ds, record_reader &rdr);

    MLIO_HIDDEN
    void
    skip_to_header_row(record_reader &rdr);

    MLIO_HIDDEN
    void
    infer_schema(instance const &ins) final;

    MLIO_HIDDEN
    void
    infer_column_types(instance const &ins);

    MLIO_HIDDEN
    void
    set_or_validate_names(instance const &ins);

    MLIO_HIDDEN
    void
    apply_column_type_overrides();

    MLIO_HIDDEN
    void
    init_parsers_and_schema();

    MLIO_HIDDEN
    bool
    should_skip(std::size_t index, std::string const &name) const noexcept;

    MLIO_HIDDEN
    intrusive_ptr<example>
    decode(instance_batch const &batch) const final;

    MLIO_HIDDEN
    std::vector<intrusive_ptr<tensor>>
    make_tensors(std::size_t batch_size) const;

private:
    csv_params params_;
    std::vector<std::string> column_names_;
    std::vector<data_type> column_types_{};
    std::vector<int> column_skips_{};
    std::vector<parser> column_parsers_{};
    intrusive_ptr<schema> schema_{};
};

/// @}

}  // namespace v1
}  // namespace mlio
