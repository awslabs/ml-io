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

#include "module.h"

#include <pybind11/stl_bind.h>

#include <exception>

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

class Py_data_iterator {
public:
    explicit Py_data_iterator(Data_reader &reader, py::object parent)
        : reader_{&reader}, parent_{std::move(parent)}
    {}

public:
    Intrusive_ptr<Example> next()
    {
        Intrusive_ptr<Example> example{};
        {
            py::gil_scoped_release rel_gil;

            example = reader_->read_example();
        }

        if (example == nullptr) {
            throw py::stop_iteration();
        }

        return example;
    }

private:
    Data_reader *reader_;
    py::object parent_;
};

class Py_data_reader : public Data_reader {
public:
    Intrusive_ptr<Example> read_example() override;

    Intrusive_ptr<Example> peek_example() override;

    Intrusive_ptr<const Schema> read_schema() override;

    void reset() noexcept override;

public:
    std::size_t num_bytes_read() const noexcept override;
};

Intrusive_ptr<Example> Py_data_reader::read_example()
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(Intrusive_ptr<Example>, Data_reader, read_example, )
}

Intrusive_ptr<Example> Py_data_reader::peek_example()
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(Intrusive_ptr<Example>, Data_reader, peek_example, )
}

Intrusive_ptr<const Schema> Py_data_reader::read_schema()
{
    // NOLINTNEXTLINE
    PYBIND11_OVERLOAD_PURE(Intrusive_ptr<const Schema>, Data_reader, read_schema, )
}

void Py_data_reader::reset() noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(void, Data_reader, reset, )
    }
    catch (...) {
        std::terminate();
    }
}

std::size_t Py_data_reader::num_bytes_read() const noexcept
{
    try {
        // NOLINTNEXTLINE
        PYBIND11_OVERLOAD_PURE(std::size_t, Data_reader, num_bytes_read, )
    }
    catch (...) {
        std::terminate();
    }
}

Data_reader_params make_data_reader_params(std::vector<Intrusive_ptr<Data_store>> dataset,
                                           std::size_t batch_size,
                                           std::size_t num_prefetched_examples,
                                           std::size_t num_parallel_reads,
                                           Last_example_handling last_example_handling,
                                           Bad_example_handling bad_example_handling,
                                           bool warn_bad_instances,
                                           std::size_t num_instances_to_skip,
                                           std::optional<std::size_t> num_instances_to_read,
                                           std::size_t shard_index,
                                           std::size_t num_shards,
                                           std::optional<float> sample_ratio,
                                           bool shuffle_instances,
                                           std::size_t shuffle_window,
                                           std::optional<std::size_t> shuffle_seed,
                                           bool reshuffle_each_epoch)
{
    Data_reader_params params{};

    params.dataset = std::move(dataset);
    params.batch_size = batch_size;
    params.num_prefetched_examples = num_prefetched_examples;
    params.num_parallel_reads = num_parallel_reads;
    params.last_example_handling = last_example_handling;
    params.bad_example_handling = bad_example_handling;
    params.warn_bad_instances = warn_bad_instances;
    params.num_instances_to_skip = num_instances_to_skip;
    params.num_instances_to_read = num_instances_to_read;
    params.shard_index = shard_index;
    params.num_shards = num_shards;
    params.sample_ratio = sample_ratio;
    params.shuffle_instances = shuffle_instances;
    params.shuffle_window = shuffle_window;
    params.shuffle_seed = shuffle_seed;
    params.reshuffle_each_epoch = reshuffle_each_epoch;

    return params;
}

Csv_params make_csv_reader_params(std::vector<std::string> column_names,
                                  std::string name_prefix,
                                  std::unordered_set<std::string> use_columns,
                                  std::unordered_set<std::size_t> use_columns_by_index,
                                  std::optional<Data_type> default_data_type,
                                  std::unordered_map<std::string, Data_type> column_types,
                                  std::unordered_map<std::size_t, Data_type> column_types_by_index,
                                  std::optional<std::size_t> header_row_index,
                                  bool has_single_header,
                                  bool dedupe_column_names,
                                  char delimiter,
                                  char quote_char,
                                  std::optional<char> comment_char,
                                  bool allow_quoted_new_lines,
                                  bool skip_blank_lines,
                                  std::optional<std::string> encoding,
                                  std::optional<std::size_t> max_field_length,
                                  Max_field_length_handling max_field_length_handling,
                                  std::optional<std::size_t> max_line_length,
                                  std::optional<Parser_params> parser_params)
{
    Csv_params csv_params{};

    csv_params.column_names = std::move(column_names);
    csv_params.name_prefix = std::move(name_prefix);
    csv_params.use_columns = std::move(use_columns);
    csv_params.use_columns_by_index = std::move(use_columns_by_index);
    csv_params.default_data_type = default_data_type;
    csv_params.column_types = std::move(column_types);
    csv_params.column_types_by_index = std::move(column_types_by_index);
    csv_params.header_row_index = header_row_index;
    csv_params.has_single_header = has_single_header;
    csv_params.dedupe_column_names = dedupe_column_names;
    csv_params.delimiter = delimiter;
    csv_params.quote_char = quote_char;
    csv_params.comment_char = comment_char;
    csv_params.allow_quoted_new_lines = allow_quoted_new_lines;
    csv_params.skip_blank_lines = skip_blank_lines;
    if (encoding) {
        csv_params.encoding = Text_encoding{std::move(encoding.value())};
    }
    csv_params.max_field_length = max_field_length;
    csv_params.max_field_length_handling = max_field_length_handling;
    csv_params.max_line_length = max_line_length;
    if (parser_params) {
        csv_params.parser_params = std::move(parser_params.value());
    }

    return csv_params;
}

Image_reader_params make_image_reader_params(Image_frame image_frame,
                                             std::optional<size_t> resize,
                                             std::vector<std::size_t> image_dimensions,
                                             bool to_rgb)
{
    Image_reader_params img_params{};
    img_params.image_frame = image_frame;
    img_params.resize = resize;
    img_params.image_dimensions = std::move(image_dimensions);
    img_params.to_rgb = to_rgb;
    return img_params;
}

Parser_params make_parser_params(std::unordered_set<std::string> nan_values, int base)
{
    Parser_params parser_params{};

    parser_params.nan_values = std::move(nan_values);
    parser_params.base = base;

    return parser_params;
}

Intrusive_ptr<Csv_reader>
make_csv_reader(Data_reader_params params, std::optional<Csv_params> csv_params)
{
    if (csv_params) {
        return make_intrusive<Csv_reader>(std::move(params), std::move(csv_params.value()));
    }

    return make_intrusive<Csv_reader>(std::move(params));
}

Intrusive_ptr<Image_reader>
make_image_reader(Data_reader_params params, Image_reader_params img_params)
{
    return make_intrusive<Image_reader>(std::move(params), std::move(img_params));
}

Intrusive_ptr<Recordio_protobuf_reader> make_recordio_protobuf_reader(Data_reader_params params)
{
    return make_intrusive<Recordio_protobuf_reader>(std::move(params));
}

Intrusive_ptr<Text_line_reader> make_text_line_reader(Data_reader_params params)
{
    return make_intrusive<Text_line_reader>(std::move(params));
}

}  // namespace

void register_data_readers(py::module &m)
{
    py::enum_<Last_example_handling>(
        m,
        "LastExampleHandling",
        "Specifies how the last ``Example`` read from a dataset should to be "
        "handled if the dataset size is not evenly divisible by the batch "
        "size.")
        .value("NONE",
               Last_example_handling::none,
               "Return an ``Example`` where the size of the batch dimension is "
               "less than the requested batch size.")
        .value("DROP", Last_example_handling::drop, "Drop the last ``Example``.")
        .value("DROP_WARN", Last_example_handling::drop_warn, "Drop the last ``Example`` and warn.")
        .value("PAD",
               Last_example_handling::pad,
               "Pad the feature tensors with zero so that the size of the batch "
               "dimension equals the requested batch size.")
        .value("PAD_WARN",
               Last_example_handling::pad_warn,
               "Pad the feature tensors with zero so that the size of the batch "
               "dimension equals the requested batch size and warn.");

    py::enum_<Bad_example_handling>(
        m,
        "BadExampleHandling",
        "Specifies how an ``Example`` that contains erroneous data should be"
        "handled.")
        .value("ERROR", Bad_example_handling::error, "Raise an error.")
        .value("SKIP", Bad_example_handling::skip, "Skip the ``Example``.")
        .value("SKIP_WARN", Bad_example_handling::skip_warn, "Skip the ``Example`` and warn.")
        .value("PAD",
               Bad_example_handling::pad,
               "Skip bad instances, pad the ``Example`` to the batch size.")
        .value("PAD_WARN",
               Bad_example_handling::pad_warn,
               "Skip bad instances, pad the ``Example`` to the batch size, and warn.");

    py::enum_<Max_field_length_handling>(
        m,
        "MaxFieldLengthHandling",
        "Specifies how field and columns should be handled when breached.")
        .value("TREAT_AS_BAD",
               Max_field_length_handling::treat_as_bad,
               "Treat the corresponding row as bad.")
        .value("TRUNCATE", Max_field_length_handling::truncate, "Truncate the field.")
        .value("TRUNCATE_WARN",
               Max_field_length_handling::truncate_warn,
               "Truncate the field and warn.");

    py::enum_<Image_frame>(m, "ImageFrame", "Specifies the Image_frame parameter value")
        .value("NONE", Image_frame::none, "none.")
        .value("RECORDIO", Image_frame::recordio, "For recordio files.");

    py::class_<Py_data_iterator>(m, "DataIterator")
        .def("__iter__",
             [](Py_data_iterator &it) -> Py_data_iterator & {
                 return it;
             })
        .def("__next__", &Py_data_iterator::next);

    py::class_<Data_reader_params>(
        m, "DataReaderParams", "Represents the common parameters of a ``Data_reader`` object.")
        .def(py::init(&make_data_reader_params),
             "dataset"_a,
             "batch_size"_a,
             "num_prefetched_examples"_a = 0,
             "num_parallel_reads"_a = 0,
             "last_example_handling"_a = Last_example_handling::none,
             "bad_example_handling"_a = Bad_example_handling::error,
             "warn_bad_instances"_a = false,
             "num_instances_to_skip"_a = 0,
             "num_instances_to_read"_a = std::nullopt,
             "shard_index"_a = 0,
             "num_shards"_a = 0,
             "sample_ratio"_a = std::nullopt,
             "shuffle_instances"_a = false,
             "shuffle_window"_a = 0,
             "shuffle_seed"_a = std::nullopt,
             "reshuffle_each_epoch"_a = true,
             R"(
            Parameters
            ----------
            dataset : list of DataStores
                A list of ``DataStore`` instances that together form the
                dataset to read from.
            batch_size : int
                A number indicating how many data instances should be packed
                into a single ``Example``.
            num_prefetched_examples : int, optional
                The number of examples to prefetch in background to accelerate
                reading. If zero, default to the number of processor cores.
            num_parallel_reads : int, optional
                The number of parallel reads. If not specified, it equals
                to `num_prefetched_examples`. In case a large number of examples
                should be prefetched, this parameter can be used to avoid
                thread oversubscription.
            last_example_handling : LastExampleHandling
                See ``LastExampleHandling``.
            bad_example_handling : BadExampleHandling
                See ``BadExampleHandling``.
            warn_bad_instances : bool, optional
                A boolean value indicating whether a warning will be output for
                each bad Instance.
            num_instances_to_skip : int, optional
                The number of data instances to skip from the beginning of the
                dataset.
            num_instances_to_read : int, optional
                The number of data instances to read. The rest of the dataset
                will be ignored.
            shard_index : int, optional
                The index of the shard to read.
            num_shards : int, optional
                The number of shards the dataset should be split into. The
                reader will only read 1/num_shards of the dataset.
            sample_ratio : float, optional
                A ratio between zero and one indicating how much of the dataset
                should be read. The dataset will be sampled based on this
                number.
            shuffle_instances : bool
                A boolean value indicating whether to shuffle the data instances
                while reading from the dataset.
            shuffle_window : int
                The number of data instances to buffer and sample from. The
                selected data instances will be replaced with new data instances
                read from the dataset.

                A value of zero means perfect shuffling and requires loading the
                whole dataset into memory first.
            shuffle_seed : int, optional
                The seed that will be used for initializing the sampling
                distribution. If not specified, a random seed will be generated
                internally.
            reshuffle_each_epoch : bool, optional
                A boolean value indicating whether the dataset should be
                reshuffled after every `Data_reader.reset()` call.
            )")
        .def_readwrite("dataset", &Data_reader_params::dataset)
        .def_readwrite("batch_size", &Data_reader_params::batch_size)
        .def_readwrite("num_prefetched_examples", &Data_reader_params::num_prefetched_examples)
        .def_readwrite("num_parallel_reads", &Data_reader_params::num_parallel_reads)
        .def_readwrite("last_example_handling", &Data_reader_params::last_example_handling)
        .def_readwrite("bad_example_handling", &Data_reader_params::bad_example_handling)
        .def_readwrite("num_instances_to_skip", &Data_reader_params::num_instances_to_skip)
        .def_readwrite("num_instances_to_read", &Data_reader_params::num_instances_to_read)
        .def_readwrite("shard_index", &Data_reader_params::shard_index)
        .def_readwrite("num_shards", &Data_reader_params::num_shards)
        .def_readwrite("sample_ratio", &Data_reader_params::sample_ratio)
        .def_readwrite("shuffle_instances", &Data_reader_params::shuffle_instances)
        .def_readwrite("shuffle_window", &Data_reader_params::shuffle_window)
        .def_readwrite("shuffle_seed", &Data_reader_params::shuffle_seed)
        .def_readwrite("reshuffle_each_epoch", &Data_reader_params::reshuffle_each_epoch);

    py::class_<Csv_params>(
        m, "CsvParams", "Represents the optional parameters of a ``CsvReader`` object.")
        .def(py::init(&make_csv_reader_params),
             "column_names"_a = std::vector<std::string>{},
             "name_prefix"_a = "",
             "use_columns"_a = std::unordered_set<std::string>{},
             "use_columns_by_index"_a = std::unordered_set<std::size_t>{},
             "default_data_type"_a = std::nullopt,
             "column_types"_a = std::unordered_map<std::string, Data_type>{},
             "column_types_by_index"_a = std::unordered_map<std::size_t, Data_type>{},
             "header_row_index"_a = 0,
             "has_single_header"_a = false,
             "dedupe_column_names"_a = true,
             "delimiter"_a = ',',
             "quote_char"_a = '"',
             "comment_char"_a = std::nullopt,
             "allow_quoted_new_lines"_a = false,
             "skip_blank_lines"_a = true,
             "encoding"_a = std::nullopt,
             "max_field_length"_a = std::nullopt,
             "max_field_length_handling"_a = Max_field_length_handling::treat_as_bad,
             "max_line_length"_a = std::nullopt,
             "Parser_params"_a = std::nullopt,
             R"(
            Parameters
            ----------
            column_names : list of strs
                The column names.

                If the dataset has a header and `header_row_index` is specified,
                this list can be left empty to infer the column names from the
                dataset.

                Due to a shortcoming in pybind11, values cannot be added to
                container types, and updates must instead be made via
                assignment.
            name_prefix : str
                The prefix to prepend to column names.
            use_columns : list of strs
                The columns that should be read. The rest of the columns will
                be skipped.

                Due to a shortcoming in pybind11, values cannot be added to
                container types, and updates must instead be made via
                assignment.
            use_columns_by_index : list of ints
                The columns, specified by index, that should be read. The rest
                of the columns will be skipped.

                Due to a shortcoming in pybind11, values cannot be added to
                container types, and updates must instead be made via
                assignment.
            default_data_type : DataType
                The data type for columns for which no explicit data type is
                specified via `column_types` or `column_types_by_index`. If not
                specified, the column data types will be inferred from the
                dataset.
            column_types : map of str/data type
                The mapping between columns and data types by name.

                Due to a shortcoming in pybind11, values cannot be added to
                container types, and updates must instead be made via
                assignment.
            column_types_by_index : map of str/int
                The mapping between columns and data types by index.

                Due to a shortcoming in pybind11, values cannot be added to
                container types, and updates must instead be made via
                assignment.
            header_row_index : int, optional
                The index of the row that should be treated as the header of the
                dataset. If `column_names` is empty, the column names will be
                inferred from that row.  If neither `header_row_index` nor
                `column_names` is specified, the column ordinal positions
                will be used as column names.

                Each data store in the dataset should have its header at the
                same index.
            has_single_header : bool, optional
                A boolean value indicating whether the dataset has a header row
                only in the first data store.
            dedupe_column_names: bool, optional
                A boolean value indicating whether duplicate columns should be
                renamed. If true, duplicate columns 'X', ..., 'X' will be
                renamed to 'X', 'X_1', X_2', ...
            delimiter : char
                The delimiter character.
            quote_char : char
                The character used for quoting field values.
            comment_char : char, optional
                The comment character. Lines that start with the comment
                character will be skipped.
            allow_quoted_new_lines : bool
                A boolean value indicating whether quoted fields can be multi-
                line. Note that turning this flag on can slow down the reading
                speed.
            skip_blank_lines : bool
                A boolean value indicating whether to skip empty lines.
            encoding : str, optional
                The text encoding to use for reading. If not specified, it will
                be inferred from the preamble of the text; otherwise falls back
                to UTF-8.
            max_field_length : int, optional
                The maximum number of characters that will be read in a field.
                Any characters beyond this limit will be handled using the
                strategy in `Max_field_length_handling`.
            max_field_length_handling : MaxFieldLengthHandling, optional
                See ``MaxFieldLengthHandling``.
            max_line_length : int, optional
                The maximum size of a text line. If a row is longer than the
                specified size, an error will be raised.
            Parser_params : ParserParams, optional
                See ``ParserParams``.
            )")
        .def_readwrite("column_names", &Csv_params::column_names)
        .def_readwrite("name_prefix", &Csv_params::name_prefix)
        .def_readwrite("use_columns", &Csv_params::use_columns)
        .def_readwrite("use_columns_by_index", &Csv_params::use_columns_by_index)
        .def_readwrite("default_data_type", &Csv_params::default_data_type)
        .def_readwrite("column_types", &Csv_params::column_types)
        .def_readwrite("column_types_by_index", &Csv_params::column_types_by_index)
        .def_readwrite("header_row_index", &Csv_params::header_row_index)
        .def_readwrite("has_single_header", &Csv_params::has_single_header)
        .def_readwrite("dedupe_column_names", &Csv_params::dedupe_column_names)
        .def_readwrite("delimiter", &Csv_params::delimiter)
        .def_readwrite("quote_char", &Csv_params::quote_char)
        .def_readwrite("comment_char", &Csv_params::comment_char)
        .def_readwrite("allow_quoted_new_lines", &Csv_params::allow_quoted_new_lines)
        .def_readwrite("skip_blank_lines", &Csv_params::skip_blank_lines)
        .def_readwrite("encoding", &Csv_params::encoding)
        .def_readwrite("max_field_length", &Csv_params::max_field_length)
        .def_readwrite("max_field_length_handling", &Csv_params::max_field_length_handling)
        .def_readwrite("max_line_length", &Csv_params::max_line_length)
        .def_readwrite("Parser_params", &Csv_params::parser_params);

    py::class_<Image_reader_params>(
        m, "ImageReaderParams", "Represents the optional parameters of an ``ImageReader`` object.")
        .def(py::init(&make_image_reader_params),
             "image_frame"_a = Image_frame::none,
             "resize"_a = std::nullopt,
             "image_dimensions"_a = std::nullopt,
             "to_rgb"_a = false,
             R"(
            Parameters
            ----------
            image_frame : enum {NONE, RECORDIO}
                Selects the image frame to NONE(for raw image files) or
                RECORDIO(for recordio files).
            resize : int, optional
                Scales the shorter edge to a new size before applying other
                augmentations.
            image_dimensions : list of ints
                The dimensions of output image in (channels, height, width)
                format.
            to_rgb : boolean
                Converts from BGR (OpenCV default) to RGB, if set to true.
            )")
        .def_readwrite("image_frame", &Image_reader_params::image_frame)
        .def_readwrite("resize", &Image_reader_params::resize)
        .def_readwrite("image_dimensions", &Image_reader_params::image_dimensions)
        .def_readwrite("to_rgb", &Image_reader_params::to_rgb);

    py::class_<Parser_params>(m, "ParserParams")
        .def(py::init(&make_parser_params),
             "nan_values"_a = std::unordered_set<std::string>{},
             "number_base"_a = 10,
             R"(
            Parameters
            ----------
            nan_values : list of strs
                For a floating-point parse operation holds the list of strings
                that should be treated as NaN.

                Due to a shortcoming in pybind11, values cannot be added to
                container types, and updates must instead be made via
                assignment.
            number_base : int
                For a number parse operation specifies the base of the number
                in its string representation.
             )")
        .def_readwrite("nan_values", &Parser_params::nan_values)
        .def_readwrite("number_base", &Parser_params::base);

    py::class_<Data_reader, Py_data_reader, Intrusive_ptr<Data_reader>>(
        m,
        "DataReader",
        "Represents an interface for classes that read examples from a "
        "dataset in a particular data format.")
        .def(py::init<>())
        .def("read_example",
             &Data_reader::read_example,
             py::call_guard<py::gil_scoped_release>(),
             "Returns the next ``Example`` read from the dataset. If the end "
             "of the data is reached, returns None")
        .def("peek_example",
             &Data_reader::peek_example,
             py::call_guard<py::gil_scoped_release>(),
             "Returns the next ``Example`` read from the dataset without "
             "consuming it.")
        .def("read_schema",
             &Data_reader::read_schema,
             py::call_guard<py::gil_scoped_release>(),
             "Returns the ``Schema`` of the dataset.")
        .def("reset",
             &Data_reader::reset,
             "Resets the state of the reader. Calling ``read_example()`` the "
             "next time will start reading from the beginning of the dataset.")
        .def("__iter__",
             [](py::object &reader) {
                 return Py_data_iterator(reader.cast<Data_reader &>(), reader);
             })
        .def_property_readonly("num_bytes_read",
                               &Data_reader::num_bytes_read,
                               R"(
             Gets the number of bytes read from the dataset.

             The returned number won't include the size of the discarded
             parts of the dataset such as comment blocks.

             The returned number can be greater than expected as MLIO
             reads ahead the dataset in background.)");

    py::class_<Csv_reader, Data_reader, Intrusive_ptr<Csv_reader>>(
        m, "CsvReader", "Represents a ``Data_reader`` for reading CSV datasets.")
        .def(py::init<>(&make_csv_reader),
             "Data_reader_params"_a,
             "Csv_params"_a = std::nullopt,
             R"(
            Parameters
            ----------
            Data_reader_params : DataReaderParams
                See ``DataReaderParams``.
            csv_reader_params : CsvReaderParams, optional
                See ``CsvReaderParams``.
            )");

    py::class_<Image_reader, Data_reader, Intrusive_ptr<Image_reader>>(
        m, "ImageReader", "Represents a ``Data_reader`` for reading Image datasets.")
        .def(py::init<>(&make_image_reader),
             "Data_reader_params"_a,
             "Image_reader_params"_a = std::nullopt,
             R"(
            Parameters
            ----------
            Data_reader_params : DataReaderParams
                See ``DataReaderParams``.
            Image_reader_params : ImageReaderParams
                See ``ImageReaderParams``.
            )");

    py::class_<Recordio_protobuf_reader, Data_reader, Intrusive_ptr<Recordio_protobuf_reader>>(
        m, "RecordIOProtobufReader")
        .def(py::init<>(&make_recordio_protobuf_reader),
             "Data_reader_params"_a,
             R"(
            Parameters
            ----------
            Data_reader_params : DataReaderParams
                See ``DataReaderParams``.
            )");

    py::class_<Text_line_reader, Data_reader, Intrusive_ptr<Text_line_reader>>(m, "TextLineReader")
        .def(py::init<>(&make_text_line_reader),
             "Data_reader_params"_a,
             R"(
            Parameters
            ----------
            Data_reader_params : DataReaderParams
                See ``DataReaderParams``.
            )");
}

}  // namespace pymlio
