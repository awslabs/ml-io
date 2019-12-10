# Record Readers
* [Classes](#RecordReader)
    * [RecordReader](#RecordReader)
    * [ParquetRecordReader](#ParquetRecordReader)
    * [Record](#Record)
* [Enumerations](#Enumerations)
    * [RecordKind](#RecordKind)
* [Exceptions](#Exceptions)

Record readers are relatively low-level contstructs in the ML-IO API. They allow reading raw binary records from an [`InputStream`](stream.md#InputStream) instance.

As of today the only publicly available record reader type is [`ParquetRecordReader`](#ParquetRecordReader) which reads Parquet files as memory blobs that can be passed to Apache Arrow.

## RecordReader
`RecordReader` is the abstract base class for record reader types.

### Methods
#### read_record
Reads the next [`Record`](#Record) from the underlying [`InputStream`](stream.md#InputStream). If the end of the stream is reached, returns `None`.

```python
read_record()
```

#### peek_record
Peeks the next [`Record`](#Record) from the underlying [`InputStream`](stream.md#InputStream) without consuming it. Calling `read_record` afterwards will return the same record.

```python
peek_record()
```

#### \_\_iter\_\_
All record readers are iterable and can be used in contexts such as `for` loops, list comprehensions, and generator expressions.

## ParquetRecordReader
`ParquetRecordReader` reads Parquet files from an underlying [`InputStream`](stream.md#InputStream) and returns them as binary blobs via [`Record`](#Record) instances. Inherits from [RecordReader](#RecordReader).

```python
ParquetRecordReader(strm : InputStream)
```
- `strm`: The input stream from which to read the Parquet files.

This class is meant to be used with input streams that can potentially contain more than one Parquet file. For example a [`SageMakerPipe`](data_store.md#SageMakerPipe) data store pointing to an S3 location with more than one Parquet file should use `ParquetRecordReader` to extract them from the input stream.

Conventional data stores such as [`File`](data_store.md#File)s don't need to use `ParquetRecordReader`. A data store containing only a single Parquet file can be directly converted into an Arrow file via `mlio.integ.arrow.as_arrow_file()` function.

## Record
A [`Record`](#Record) is a binary blob containing the raw bytes of a data instance and supports the Python Buffer protocol.

### Properties
#### kind
Gets the kind of the record; indicating whether it is a complete or a partial record.

## Enumerations
### RecordKind
Specifies the kind of a record.

| Value      | Description                                           |
|------------|-------------------------------------------------------|
| `COMPLETE` | The record contains a complete data instance.         |
| `BEGIN`    | The record contains the beginning of a data instance. |
| `MIDDLE`   | The record contains the middle of a data instance.    |
| `END`      | The record contains the end of a data instance.       |

## Exceptions
| Type                  | Description                                                                       |
|-----------------------|-----------------------------------------------------------------------------------|
| `CorruptRecordReader` | Thrown when the record is corrupt. Inherits from `RuntimeError`                   |
| `CorruptRecordHeader` | Thrown when the record has a corrupt header. Inherits from `CorruptRecordReader`. |
| `CorruptRecordFooter` | Thrown when the record has a corrupt footer. Inherits from `CorruptRecordReader`. |
| `RecordTooLargeError` | Thrown when the record is larger than a threshold. Inherits from `RuntimeError`   |
