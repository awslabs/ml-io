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

#include "core/module.h"

#include <pybind11/stl_bind.h>

#include <exception>

namespace py = pybind11;
namespace stdx = mlio::stdx;

using namespace pybind11::literals;

namespace mliopy {
namespace detail {
namespace {

class py_data_iterator {
public:
    explicit
    py_data_iterator(mlio::data_reader &rdr, py::object parent)
        : reader_{&rdr}, parent_{std::move(parent)}
    {}

public:
    mlio::intrusive_ptr<mlio::example>
    next()
    {
        mlio::intrusive_ptr<mlio::example> exm = reader_->read_example();
        if (exm == nullptr) {
            throw py::stop_iteration();
        }

        return exm;
    }

private:
    mlio::data_reader *reader_;
    py::object parent_;
};

class py_data_reader : public mlio::data_reader {
public:
    mlio::intrusive_ptr<mlio::example>
    read_example() override;

    mlio::intrusive_ptr<mlio::example> const &
    peek_example() override;

    void
    reset() noexcept override;
};

mlio::intrusive_ptr<mlio::example>
py_data_reader::
read_example()
{
    PYBIND11_OVERLOAD_PURE(mlio::intrusive_ptr<mlio::example>,
                           mlio::data_reader,
                           read_example,)
}

mlio::intrusive_ptr<mlio::example> const &
py_data_reader::
peek_example()
{
    PYBIND11_OVERLOAD_PURE(mlio::intrusive_ptr<mlio::example> const &,
                           mlio::data_reader,
                           peek_example,)
}

void
py_data_reader::
reset() noexcept
{
    try {
        PYBIND11_OVERLOAD_PURE(void, mlio::data_reader, reset,)
    }
    catch (...) {
        std::terminate();
    }
}

mlio::intrusive_ptr<mlio::csv_reader>
make_csv_reader(
    std::vector<mlio::intrusive_ptr<mlio::data_store>> dataset,
    std::size_t batch_size,
    std::size_t num_prefetched_batches,
    std::size_t num_parallel_reads,
    mlio::last_batch_handling last_batch_hnd,
    mlio::bad_batch_handling bad_batch_hnd,
    std::size_t num_instances_to_skip,
    stdx::optional<std::size_t> num_instances_to_read,
    bool shuffle_instances,
    std::size_t shuffle_window,
    stdx::optional<std::size_t> shuffle_seed,
    std::vector<std::string> column_names,
    std::string name_prefix,
    std::unordered_set<std::string> use_columns,
    std::unordered_set<std::size_t> use_columns_by_index,
    stdx::optional<mlio::data_type> default_data_type,
    std::unordered_map<std::string, mlio::data_type> column_types,
    std::unordered_map<std::size_t, mlio::data_type> column_types_by_index,
    stdx::optional<std::size_t> header_row_index,
    char delimiter,
    stdx::optional<char> comment_char,
    bool allow_quoted_new_lines,
    bool skip_blank_lines,
    std::string encoding,
    std::unordered_set<std::string> nan_values,
    int base)
{
    mlio::data_reader_params rdr_prm{};

    rdr_prm.dataset = std::move(dataset);
    rdr_prm.batch_size = batch_size;
    rdr_prm.num_prefetched_batches = num_prefetched_batches;
    rdr_prm.num_parallel_reads = num_parallel_reads;
    rdr_prm.last_batch_hnd = last_batch_hnd;
    rdr_prm.bad_batch_hnd = bad_batch_hnd;
    rdr_prm.num_instances_to_skip = num_instances_to_skip;
    rdr_prm.num_instances_to_read = num_instances_to_read;  // NOLINT
    rdr_prm.shuffle_instances = shuffle_instances;
    rdr_prm.shuffle_window = shuffle_window;
    rdr_prm.shuffle_seed = shuffle_seed;  // NOLINT


    mlio::csv_params csv_prm{};

    csv_prm.column_names = std::move(column_names);
    csv_prm.name_prefix = std::move(name_prefix);
    csv_prm.use_columns = std::move(use_columns);
    csv_prm.use_columns_by_index = std::move(use_columns_by_index);
    csv_prm.default_data_type = default_data_type;  // NOLINT
    csv_prm.column_types = std::move(column_types);
    csv_prm.column_types_by_index = std::move(column_types_by_index);
    csv_prm.header_row_index = header_row_index;  // NOLINT
    csv_prm.delimiter = delimiter;
    csv_prm.comment_char = comment_char;  // NOLINT
    csv_prm.allow_quoted_new_lines = allow_quoted_new_lines;
    csv_prm.skip_blank_lines = skip_blank_lines;
    if (!encoding.empty()) {
        csv_prm.encoding = mlio::text_encoding{std::move(encoding)};
    }
    csv_prm.parser_prm.nan_values = std::move(nan_values);
    csv_prm.parser_prm.base = base;

    return mlio::make_intrusive<mlio::csv_reader>(std::move(rdr_prm), std::move(csv_prm));
}

mlio::intrusive_ptr<mlio::recordio_protobuf_reader>
make_recordio_protobuf_reader(
    std::vector<mlio::intrusive_ptr<mlio::data_store>> dataset,
    std::size_t batch_size,
    std::size_t num_prefetched_batches,
    std::size_t num_parallel_reads,
    mlio::last_batch_handling last_batch_hnd,
    mlio::bad_batch_handling bad_batch_hnd,
    std::size_t num_instances_to_skip,
    stdx::optional<std::size_t> num_instances_to_read,
    bool shuffle_instances,
    std::size_t shuffle_window,
    stdx::optional<std::size_t> shuffle_seed)
{
    mlio::data_reader_params rdr_prm{};

    rdr_prm.dataset = std::move(dataset);
    rdr_prm.batch_size = batch_size;
    rdr_prm.num_prefetched_batches = num_prefetched_batches;
    rdr_prm.num_parallel_reads = num_parallel_reads;
    rdr_prm.last_batch_hnd = last_batch_hnd;
    rdr_prm.bad_batch_hnd = bad_batch_hnd;
    rdr_prm.num_instances_to_skip = num_instances_to_skip;
    rdr_prm.num_instances_to_read = num_instances_to_read;  // NOLINT
    rdr_prm.shuffle_instances = shuffle_instances;
    rdr_prm.shuffle_window = shuffle_window;
    rdr_prm.shuffle_seed = shuffle_seed;  // NOLINT

    return mlio::make_intrusive<mlio::recordio_protobuf_reader>(std::move(rdr_prm));
}

}  // namespace
}  // namespace detail

void
register_data_readers(py::module &m)
{
    py::enum_<mlio::last_batch_handling>(m, "LastBatchHandling",
        "Specifies how the last batch read from a dataset should to be "
        "handled if the dataset size is not evenly divisible by the batch "
        "size.")
        .value("NONE", mlio::last_batch_handling::none,
            "Return an ``example`` where the size of the batch dimension is "
            "less than the requested batch size.")
        .value("DROP", mlio::last_batch_handling::drop,
            "Drop the last ``example``.")
        .value("PAD",  mlio::last_batch_handling::pad,
            "Pad the feature tensors with zero so that the size of the batch "
            "dimension equals the requested batch size.");

    py::enum_<mlio::bad_batch_handling>(m, "BadBatchHandling",
        "Specifies how a batch that contains erroneous data should be"
        "handled.")
        .value("ERROR", mlio::bad_batch_handling::error,
            "Throw an exception.")
        .value("SKIP",  mlio::bad_batch_handling::skip,
            "Skip the batch.")
        .value("WARN",  mlio::bad_batch_handling::warn,
            "Skip the batch and log a warning message.");

    py::class_<detail::py_data_iterator>(m, "DataIterator")
        .def("__iter__", [](detail::py_data_iterator &it) -> detail::py_data_iterator &
            {
                return it;
            })
        .def("__next__", &detail::py_data_iterator::next);

    py::class_<mlio::data_reader,
               detail::py_data_reader,
               mlio::intrusive_ptr<mlio::data_reader>>(m, "DataReader",
        "Represents an interface for classes that read examples from a "
        "dataset in a particular data format."
        )
        .def(py::init<>())
        .def("read_example", &mlio::data_reader::read_example,
            py::call_guard<py::gil_scoped_release>(),
            "Returns the next ``example`` read from the dataset. If the end "
            "of the data is reached, returns None")
        .def("peek_example", &mlio::data_reader::peek_example,
            py::call_guard<py::gil_scoped_release>(),
            "Returns the next ``example`` read from the dataset without "
            "consuming it.")
        .def("reset", &mlio::data_reader::reset,
            "Resets the state of the reader. Calling ``read_example()`` the "
            "next time will start reading from the beginning of the dataset.")
        .def("__iter__", [](py::object &rdr)
            {
                return detail::py_data_iterator(rdr.cast<mlio::data_reader &>(), rdr);
            });

    py::class_<mlio::csv_reader,
               mlio::data_reader,
               mlio::intrusive_ptr<mlio::csv_reader>>(m, "CsvReader",
        "Represents a ``data_reader`` for reading CSV datasets.")
        .def(py::init<>(&detail::make_csv_reader),
            "dataset"_a,
            "batch_size"_a,
            "num_prefetched_batches"_a = 0,
            "num_parallel_reads"_a = 0,
            "last_batch_handling"_a = mlio::last_batch_handling::none,
            "bad_batch_handling"_a = mlio::bad_batch_handling::error,
            "num_instances_to_skip"_a = 0,
            "num_instances_to_read"_a = stdx::nullopt,
            "shuffle_instances"_a = false,
            "shuffle_window"_a = 0,
            "shuffle_seed"_a = stdx::nullopt,
            "column_names"_a = std::vector<std::string>{},
            "name_prefix"_a = "",
            "use_columns"_a = std::unordered_set<std::string>{},
            "use_columns_by_index"_a = std::unordered_set<std::size_t>{},
            "default_data_type"_a = stdx::nullopt,
            "column_types"_a = std::unordered_map<std::string, mlio::data_type>{},
            "column_types_by_index"_a = std::unordered_map<std::size_t, mlio::data_type>{},
            "header_row_index"_a = 0,
            "delimiter"_a = ',',
            "comment_char"_a = '#',
            "allow_quoted_new_lines"_a = false,
            "skip_blank_lines"_a = true,
            "encoding"_a = "",
            "nan_values"_a = std::unordered_set<std::string>{},
            "number_base"_a = 10,
            R"(
            Parameters
            ----------
            dataset : list of DataStores
                A list of ``data_store`` instances that together form the 
                dataset to read from.
            batch_size : int
                A number that indicates how many data instances should be packed
                into a single ``example``.
            num_prefetched_batches : int, optional
                The number of batches to prefetch in background to accelerate
                reading. If zero, default to the number of processor cores.
            num_parallel_reads : int, optional
                The number of parallel batch reads. If not specified, it equals
                to `num_prefetched_batche`. In case a large number of batches
                should be prefetched, this parameter can be used to avoid
                thread oversubscription.
            last_batch_handling : LastBatchHandling
                See ``LastBatchHandling``.
            bad_batch_handling : BadBatchHandling
                See ``BadBatchHandling``.
            num_instances_to_skip : int, optional
                The number of data instances to skip from the beginning of the
                dataset.
            num_instances_to_read : int, optional
                A boolean value indicating whether to shuffle the data instnaces
                while reading from the dataset.
            shuffle_instances : bool
                The number of data instances to buffer and sample from. The
                selected data instances will be replaced with new data instnaces
                read from the dataset.

                A value of zero means perfect shuffling where the whole dataset
                will be read into memory first.
            shuffle_window : int
                The seed that will be used for initializing the sampling
                distribution. If not specified, a random seed will be generated
                internally.
            header_row_index : int, optional
                The index of the row that should be treated as the header of the
                dataset. If specified, the column names will be inferred from
                that row; otherwise `column_names` will be used. If neither
                `header_row_index` nor `column_names` is specified, the column
                ordinal positions will be used as column names

                Each data store in the dataset should have its header at the
                same index.
            column_names : list of strs
                The column names; ignored if `header_row_index` is specified.
            name_prefix : str
                The prefix to prepend to column names.
            use_columns : list of strs
                The columns that should be read. The rest of the columns will
                be skipped.
            use_columns_by_index : list of ints
                The columns, specified by index, that should be read. The rest
                of the columns will be skipped.
            default_data_type : DataType
                The data type for columns for which no explicit data type is
                specified via `column_types` or `column_types_by_index`. If not
                specified, the column data types will be inferred from the
                dataset.
            column_types : map of str/data type
                The mapping between columns and data types by name.
            columnd_types_by_index : map of str/int
                The mapping between columns and data types by index.
            delimiter : char
                The delimiter character.
            comment_char : char, optional
                The comment character. Lines that start with the comment
                character will be skipped.
            allow_quoted_new_lines : bool
                A boolean value indicating whether quoted fields can be multi-
                line. Note that turning this flag on can slow down the reading
                speed.
            skip_blank_lines : bool
                A boolean value indicating whether to skip empty lines.
            encoding : str
                The text encoding to use for reading. If not specified, it will
                be inferred from the preamble of the text; otherwise falls back
                to UTF-8.
            nan_values : list of strs
                For a floating-point parse operations holds the list of strings
                that should be treated as NaN.
            number_base : int
                For a number parse operation specified the base of the number
                in its string representation.
            )");

    py::class_<mlio::recordio_protobuf_reader,
               mlio::data_reader,
               mlio::intrusive_ptr<mlio::recordio_protobuf_reader>>(m, "RecordIOProtobufReader")
        .def(py::init<>(&detail::make_recordio_protobuf_reader),
            "dataset"_a,
            "batch_size"_a,
            "num_prefetched_batches"_a = 0,
            "num_parallel_reads"_a = 0,
            "last_batch_handling"_a = mlio::last_batch_handling::none,
            "bad_batch_handling"_a = mlio::bad_batch_handling::error,
            "num_instances_to_skip"_a = 0,
            "num_instances_to_read"_a = stdx::nullopt,
            "shuffle_instances"_a = false,
            "shuffle_window"_a = 0,
            "shuffle_seed"_a = stdx::nullopt,
            R"(
            Parameters
            ----------
            dataset : list of DataStores
                A list of ``data_store`` instances that together form the 
                dataset to read from.
            batch_size : int
                A number that indicates how many data instances should be packed
                into a single ``example``.
            num_prefetched_batches : int, optional
                The number of batches to prefetch in background to accelerate
                reading. If zero, default to the number of processor cores.
            num_parallel_reads : int, optional
                The number of parallel batch reads. If not specified, it equals
                to `num_prefetched_batche`. In case a large number of batches
                should be prefetched, this parameter can be used to avoid
                thread oversubscription.
            last_batch_handling : LastBatchHandling
                See ``LastBatchHandling``.
            bad_batch_handling : BadBatchHandling
                See ``BadBatchHandling``.
            num_instances_to_skip : int, optional
                The number of data instances to skip from the beginning of the
                dataset.
            num_instances_to_read : int, optional
                A boolean value indicating whether to shuffle the data instnaces
                while reading from the dataset.
            shuffle_instances : bool
                The number of data instances to buffer and sample from. The
                selected data instances will be replaced with new data instnaces
                read from the dataset.

                A value of zero means perfect shuffling where the whole dataset
                will be read into memory first.
            shuffle_window : int
                The seed that will be used for initializing the sampling
                distribution. If not specified, a random seed will be generated
                internally.
            )");
}

}  // namespace mliopy
