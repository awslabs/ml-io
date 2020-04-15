# Data Readers
* [Classes](#DataReader)
    * [DataReader](#DataReader)
    * [CsvReader](#CsvReader)
    * [RecordIOProtobufReader](#RecordIOProtobufReader)
    * [ImageReader](#ImageReader)
    * [DataReaderParams](#DataReaderParams)
    * [CsvParams](#CsvParams)
    * [ImageReaderParams](#ImageReaderParams)
    * [ParserParams](#ParserParams)
    * [Example](#Example)
    * [Schema](#Schema)
    * [Attribute](#Attribute)
* [Enumerations](#Enumerations)
    * [LastBatchHandling](#LastBatchHandling)
    * [BadBatchHandling](#BadBatchHandling)
    * [ImageFrame](#ImageFrame)
    * [MaxFieldLengthHandling](#MaxFieldLengthHandling)
* [Exceptions](#Exceptions)

A data reader is the main interface of ML-IO for reading datasets. A dataset is a collection of one or more [data stores](data_store.md) all of which contain data in the same format (e.g. CSV or RecordIO-protobuf). By instantiating a subclass of [`DataReader`](#DataReader) such as a [`CsvReader`](#CsvReader) or a [`RecordIOProtobufReader`](#RecordIOProtobufReader) a dataset can be read in batches.

## DataReader
Represents an abstract base class for all data reader types.

### Methods
#### read_schema
Reads the [Schema](#Schema) of the underlying dataset.

```python
read_schema()
```

#### read_example
Reads the next [Example](#Example) from the underlying dataset. If the end of the dataset is reached, returns None.

```python
read_examle()
```

#### peek_example
Peeks the next [`Example`](#Example) from the underlying dataset without consuming it. Calling `read_example` afterwards will return the same example.

```python
peek_examle()
```

#### reset
Resets the state of the data reader. Calling [`read_example()`](#read_example) the next time will start reading from the beginning of the dataset.

```python
reset()
```

#### \_\_iter\_\_
All data readers are iterable and can be used in contexts such as `for` loops, list comprehensions, and generator expressions.

### Properties
#### num_bytes_read
Gets the number of bytes read from the dataset. The returned number won't include the size of the discarded parts of the dataset such as comment blocks.

> The returned number can be greater than expected as ML-IO reads ahead the dataset in background.

## CsvReader
Represents a data reader for reading CSV datasets.  Inherits from [DataReader](#DataReader).

```python
CsvReader(data_reader_params : DataReaderParams, csv_params : CsvReaderParams = None)
```

- `data_reader_params`: See [`DataReaderParams`](#DataReaderParams).
- `csv_params`: See [`CsvParams`](#CsvParams).

## RecordIOProtobufReader
Represents a data reader for reading [RecordIO-protobuf](https://docs.aws.amazon.com/sagemaker/latest/dg/cdf-training.html) datasets.

```python
RecordIOProtobufReader(data_reader_params : DataReaderParams)
```

- `data_reader_params`: See[`DataReaderParams`](#DataReaderParams).

## ImageReader
Represents a data reader for reading image datasets in JPEG and PNG formats.

```python
ImageReader(data_reader_params : DataReaderParams, image_reader_params : ImageReaderParams)
```

- `data_reader_params`: See [`DataReaderParams`](#DataReaderParams).
- `image_reader_params`: See [`ImageReaderParams`](#ImageReaderParams).

## DataReaderParams
Contains the common parameters used by all data readers.

All constructor parameters described below have a same-named read/write accessor property. Not though that, due to a shortcoming in pybind11-based language bindings, values cannot be added to container types via properties and updates must instead be made via assignment.

```python
DataReaderParams(dataset : Sequence[DataStore],
                 batch_size : int,
                 num_prefetched_batches : int = 0,
                 num_parallel_reads : int = 0,
                 last_batch_handling : LastBatchHandling = LastBatchHandling.NONE,
                 bad_batch_handling : BadBatchHandling = BatchBatchHandling.ERROR,
                 num_instances_to_skip : int = 0,
                 num_instances_to_read : Optional[int] = None,
                 shard_index : int = 0,
                 num_shards : int = 0,
                 shuffle_instances : bool = False,
                 shuffle_window : int = 0,
                 shuffle_seed : Optional[int] = None,
                 reshuffle_each_epoch : bool = True,
                 subsample_ratio: Optional[float] : None)
```

- `dataset`: A sequence of [`DataStore`](data_store.md#DataStore) instances that together form the dataset to read from.
- `batch_size`: A number indicating how many data instances should be packed into a single [`Example`](#Example).
- `num_prefetched_batches`: The number of batches to prefetch in background to accelerate reading. If zero, defaults to the number of processor cores.
- `num_parallel_reads`: The number of parallel batch reads. If not specified, it equals to `num_prefetched_batches`. In case a large number of batches should be prefetched, this parameter can be used to avoid thread oversubscription.
- `last_batch_handling`: See [`LastBatchHandling`](#LastBatchHandling).
- `bad_batch_handling`: See [`BadBatchHandling`](#BadBatchHandling).
- `num_instances_to_skip`: The number of data instances to skip from the beginning of the dataset.
- `num_instances_to_read`: The number of data instances to read. The rest of the dataset will be ignored.
- `shard_index`: The index of the shard to read.
- `num_shards`: The number of shards the dataset should be split into. The reader will only read `1/num_shards` of the dataset.
- `shuffle_instances`: A boolean value indicating whether to shuffle the data instances while reading from the dataset.
- `shuffle_window`: The number of data instances to buffer and sample from. The selected data instances will be replaced with new data instances read from the dataset. A value of zero means perfect shuffling and requires loading the whole dataset into memory first.
- `shuffle_seed`: The seed that will be used for initializing the sampling distribution. If not specified, a random seed will be generated internally.
- `reshuffle_each_epoch`: A boolean value indicating whether the dataset should be reshuffled after every [`reset()`](#reset) call.
- `subsample_ratio`: A ratio between zero and one indicating how much of the dataset (after sharding) should be read. The dataset will be subsampled based on this number. Note that, as the size of a dataset is not always known in advance, the ratio will be used as an approximation for the actual amount of data to read.

## CsvParams
Contains the parameters used by [`CsvReader`](#CsvReader).

All constructor parameters described below have a same-named read/write accessor property. Not though that, due to a shortcoming in pybind11-based language bindings, values cannot be added to container types via properties and updates must instead be made via assignment.

```python
CsvReaderParams(column_names : Sequence[str] = None,
                name_prefix : str = ""
                use_columns : Set[str] = None,
                use_columns_by_index : Set[int] = None,
                default_data_type : Optional[DataType] = None,
                column_types : Dict[str, DataType] = None,
                column_types_by_index : Dic[int, DataType] = None,
                header_row_index : Optional[int] = 0,
                has_single_header : bool = False,
                dedupe_column_names : bool = True,
                delimiter : str = ',',
                quote_char : str = '"',
                comment_char : str = None,
                allow_quoted_new_lines : bool = False,
                skip_blank_lines : bool = True,
                encoding : str = None,
                max_field_length : Optional[int] = None,
                max_field_length_handling : MaxFieldLengthHandling = MaxFieldLengthHandling.ERROR
                max_line_length : Optional[int] = None,
                parser_params : ParserParams = None)
```

- `column_names`: The colum names. If the dataset has a header and `header_row_index` is specified, this list can be left empty to infer the column names from the dataset.
- `name_prefix`: The prefix to prepend to column names.
- `use_columns`: The columns that should be read. The rest of the columns will be skipped.
- `use_columns_by_index`: The columns, specified by index, that should be read. The rest of the columns will be skipped.
- `default_data_type`: The [data type](tensor.md#DataType) for columns for which no explicit data type is specified via `column_types` or `column_types_by_index`. If not specified, the column data types will be inferred from the dataset.
- `column_types`: The mapping between columns and [data types](tensor.md#DataType) by name.
- `column_types_by_index`: The mapping between columns and [data types](tensor.md#DataType) by index.
- `header_row_index`: The index of the row that should be treated as the header of the dataset. If `column_names` is empty, the column names will be inferred from that row. If neither `header_row_index` nor `column_names` is specified, the column ordinal positions will be used as column names. Each data store in the dataset should have its header at the same index.
- `has_single_header`: A boolean value indicating whether the dataset has a header row only in the first data store.
- `dedupe_column_names`: A boolean value indicating whether duplicate columns should be renamed. If true, duplicate columns 'X', ..., 'X' will be renamed to 'X', 'X_1', 'X_2', ..., 'X_N'.
- `delimiter`: The delimiter character.
- `quote_char`: The character used for quoting field values.
- `comment_char`: The comment character. Lines that start with the comment character will be skipped.
- `allow_quoted_new_lines`: A boolean value indicating whether quoted fields can be multiline. Note that enabling this option will slow down the reading speed.
- `skip_blank_lines`: A boolean value indicating whether to skip empty lines.
- `encoding`: The text encoding to use. If not specified, it will be inferred from the preamble of the text; otherwise, falls back to UTF-8. The specified encoding should be a valid name that is accepted by `iconv(1)`.
- `max_field_length`: The maximum number of characters that will be read in a field. Any characters beyond this limit will be handled using the strategy specified in `max_field_length_handling`.
- `max_field_length_handling`: See [`MaxFieldLengthHandling`](#MaxFieldLengthHandling).
- `max_line_length`: The maximum length of a row. A row longer than this threshold will cause the data reader to fail.
- `parser_params`: See [`ParserParams`](#ParserParams).

## ImageReaderParams
Contains the parameters used by [`ImageReader`](#ImageReader).

All constructor parameters described below have a same-named read/write accessor property. Not though that, due to a shortcoming in pybind11-based language bindings, values cannot be added to container types via properties and updates must instead be made via assignment.

```python
ImageReaderParams(img_frame : ImageFrame = ImageFrame.NONE,
                  resize : Optional[int] = None,
                  image_dimensions : Sequence[int] = None,
                  to_rgb : bool = False)
```

- `img_frame`: See [`ImageFrame`](#ImageFrame)
- `resize`: Scales the shorter edge of the image to this value before applying other augmentations.
- `image_dimensions`: The dimensions of output image in `channels, height, width` format.
- `to_rgb`: A boolean value for converting from BGR (OpenCV default) to RGB color scheme.

## ParserParams
Contains the parameters used for parsing dataset features.

All constructor parameters described below have a same-named read/write accessor property. Not though that, due to a shortcoming in pybind11-based language bindings, values cannot be added to container types via properties and updates must instead be made via assignment.

```python
ParserParams(nan_values : Set[str] = None, number_base : int = 10)
```

- `nan_values`: For a floating-point parse operation holds the list of strings that should be treated as NaN.
- `number_base`: For a number parse operation specifies the base of the number in its string represetation.

## Example
Represents a batch returned by [`read_example()`](#read_example) of a data reader. It contains a collection of [`Tensor`](tensor.md#Tensor) instances corresponding to each feature in the dataset and an associated [`Schema`](#Schema) instance describing the dataset.

```python
Example(schema : Schema, features : Sequence[Tensor])
```

- `schema`: The [schema](#Schema) of the dataset.
- `features`: The data of the batch in form of [`Tensor`](tensor.md#Tensor) instances per dataset feature.

### Functions
#### \_\_len\_\_, \_\_contains\_\_, \_\_getitem\_\_
`Example` implements the Python sequence protocol. The features of an example can be retrieved by both index and name:

```python
# Either by name
lbl = example["label"]
# or by index
lbl = example[1]
```

#### \_\_iter\_\_
The features of an `Example` instance can be iterated:
```python
for feature in example:
    ...
```

### Properties
#### schema
Gets the [`Schema`](#Schema) instance describing the dataset.

#### padding
Gets the padding of the batch. If it is greater than zero, it means that the last `padding` number of elements in the batch dimension are zero-initialized. This is typically the case for the last batch read from a dataset if the size of the dataset is not evenly divisible by the batch size.

## Schema
Describes the attributes of a dataset.

```python
Schema(attrs : Sequence[Attribute])
```

- `attrs`: A sequence of [`Attribute`](#Attribute) instances describing the attributes of the dataset.

### Functions
#### get_index
Returns the index of the [attributes](#Attribute) with the specified name.

```python
get_index(name : str)
```

- `name`: The name of the attribute.

### Properties
#### descriptors
Gets the list of [attributes](#Attribute).

## Attribute
Describes an attribute which defines a measurable property of a dataset.

```python
Attribute(name : str,
          dtype : DataType,
          shape : Sequence[int],
          strides : Sequence[int] = None,
          sparse : bool = False)
```

- `name`: The name of the attribute.
- `dtype`: The [data type](tensor.md#DataType) of the attribute.
- `shape`: The shape of the attribute.
- `strides`: The strides, if any, of the attribute.
- `sparse`: A boolean value indicating whether the attribute is sparse or dense.

### Properties
#### name
Gets the name of the attribute.

#### dtype
Gets the [data type](tensor.md#DataType) of the attribute.

#### shape
Gets the strides of the attribute.

#### spase
Gets a boolean value indicating whether the attribute is sparse or dense.

## Enumerations
### LastBatchHandling
Specifies how the last batch read from a dataset should be handled if the dataset size is not evenly divisible by the batch size.

| Value  | Description                                            |
|--------|--------------------------------------------------------|
| `NONE` | Return an [`Example`](#Example) where the size of the batch dimension is less that the requested batch size. |
| `DROP` | Drop the last [`Example`](#Example). |
| `PAD`  | Pad the features of the [`Example`](#Example) with zero so that the size of the batch dimension equals the requested batch size. |

### BadBatchHandling
Specifies how a batch that contains erroneous data should be handled.

| Value   | Description                               |
|---------|-------------------------------------------|
| `ERROR` | Raise an error.                           |
| `SKIP`  | Skip the batch.                           |
| `WARN`  | Skip the batch and log a warning message. |

### ImageFrame
Specifies what image frame to use for reading an image dataset.

| Value       | Description                                        |
|-------------|----------------------------------------------------|
| `NONE`      | For reading raw image files in jpeg or png format. |
| `RECORDIO`  | For reading MXNet RecordIO based image files.      |

### MaxFieldLengthHandling
Specifies how field and columns should be handled when breached.

| Value      | Description                                   |
|------------|-----------------------------------------------|
| `ERROR`    | Raise an error.                               |
| `TRUNCATE` | Truncate the field.                           |
| `WARN`     | Truncate the field and log a warning message. |


## Exceptions
| Type                   | Description                                                                                 |
|------------------------|---------------------------------------------------------------------------------------------|
| `DataReaderError`      | Thrown when the dataset cannot be read. Inherits from `MLIOError`.                          |
| `SchemaError`          | Thrown when the dataset has a schema error. Inherits from `DataReaderError`.                |
| `InvalidInstanceError` | Thrown when the dataset contains an invalid data instance. Inherits from `DataReaderError`. |
| `FieldTooLargeError`   | Thrown when the dataset contains a field that exceeds the maximum allowed length. Inherits from `DataReaderError`. |
