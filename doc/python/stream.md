# Streams
* [Classes](#InputStream)
  * [InputStream](#InputStream)
  * [OutputStream](#OutputStream)
* [Exceptions](#Exceptions)

Input and output streams are exposed by [`DataStore`](data_store.md#DataStore) instances via [`open_read`](data_store.md#open_read) and `open_write` functions. They allow data to be read from or written to a data store in binary form. Depending on the data store a stream can be seekable and can support zero-copy reading/writing.

## InputStream
An input stream is used for reading data from an underlying [`DataStore`](data_store.md#DataStore) instance. An `InputStream` instance is usually retrieved by calling the [`open_read`](data_store.md#open_read) method of a data store.

### Methods
#### read
Expects the caller to provide a writable Python object that supports the Python Buffer protocol (e.g. `array.array`). It fills the specified buffer with data read from the underlying data store and returns the number of bytes read.

```python
read(buf : Buffer)
```
- `buf`: A writable Python object that supports the Python Buffer protocol

#### seek
Seeks to the specified position in the stream.

```python
seek(pos : int)
```

- `pos`: The new position in the stream

#### close
Closes the input stream.

```python
close()
```

#### \_\_enter\_\_ and \_\_exit\_\_
`InputStream` supports the context manager protocol and can be automatically closed via a `with` statement.

### Properties
#### size
Gets the size of the stream. Only supported by seekable streams; otherwise, raises a `NotSupportedError`.

#### seekable
Gets a boolean value indicating whether the stream is seekable.

#### supports_zero_copy
Gets a boolean value indicating whether the stream supports zero-copy reading.

#### closed
Gets a boolean value indicating whether the stream is closed.

## OutputStream
Not implemented yet

## Exceptions
| Type           | Description                                                                 |
|----------------|-----------------------------------------------------------------------------|
| `StreamError`  | Thrown when the stream cannot be read. Inherits from `RuntimeError`.        |
| `InflateError` | Thrown when the stream cannot be decompressed. Inherits from `StreamError`. |
