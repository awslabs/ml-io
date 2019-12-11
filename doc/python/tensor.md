# Tensors
* [Classes](#Tensor)
    * [Tensor](#Tensor)
    * [DenseTensor](#DenseTensor)
    * [CooTensor](#CooTensor)
    * [DeviceArray](#DeviceArray)
    * [Device](#Device)
    * [DeviceKind](#DeviceKind)
* [Enumerations](#Enumerations)
    * [DataType](#DataType)

A tensor is an in-memory representation of an n-dimensional array. It is the primary data structure used by ML-IO to expose datasets in its API. Besides the conventional dense tensors, where the whole tensor data is allocated as a single contiguous memory block, ML-IO also supports multi-dimensional sparse [COO tensors](https://en.wikipedia.org/wiki/Sparse_matrix#Coordinate_list_(COO)).

The tensor types in ML-IO are deliberately designed to be lightweight. Their primary purpose is to expose their data in most efficient way to mainstream numerical libraries and frameworks such as NumPy or PyTorch.

## Tensor
Represents an abstract base class that only defines the data type and the shape of a tensor. Derived types specify how the tensor data is laid out in memory.

### Properties
#### dtype
Gets the [data type](#DataType) of the tensor.

#### shape
Gets a `tuple` of `int`s that describes the shape of the tensor.

#### strides
Gets a `tuple` of `int`s that describes the strides of the tensor.

## DenseTensor
Represents a tensor that stores its data in a contiguous memory block. Inherits from [Tensor](#Tensor).

`DenseTensor` supports the Python Buffer protocol and can be zero-copy converted into a NumPy array or any other Python type that natively supports the Python Buffer protocol.

```python
DenseTensor(shape : Sequence[int], data : buffer, strides : Sequence[int] = None, copy : bool = True)
```
- `shape`: A sequence of `int`s that describes the shape of the tensor.
- `data`: A Python object that contains the data of the tensor and that supports the Python Buffer protocol.
- `strides`: A sequence of `int`s that describes the strides of the tensor. ML-IO can handle negative strides, but note that not all libraries have full support for strided tensors.
- `copy`: A boolean value indicating whether ML-IO should use a copy of the data contained in `data` or use `data` directly.

### Properties
#### data
Gets a [DeviceArray](#DeviceArray) that contains the tensor data.

## CooTensor
Represents a tensor that stores its data in [coordinate format](https://en.wikipedia.org/wiki/Sparse_matrix#Coordinate_list_(COO)). Inherits from [Tensor](#Tensor).

```python
CooTensor(shape : Sequence[int], data : buffer, coords : Sequence[buffer], copy : bool = True)
```

- `shape`: A sequence of `int`s that describes the shape of the tensor.
- `data`: A Python object that contains the data of the tensor and that supports the Python Buffer protocol.
- `coords`: A sequence that has the same length as the rank of the tensor (`len(shape)`) and that contains Python objects supporting the Python Buffer protocol. Each element should be a contiguous buffer of integers representing the data indices. See [here](https://en.wikipedia.org/wiki/Sparse_matrix#Coordinate_list_(COO)) for a detailed description of how `data` and `coords` together define a tensor.
- `copy`: A boolean value indicating whether ML-IO should use a copy of the data contained in `data` and `coords` or use them directly.

### Functions
#### indices
Returns a [DeviceArray](#DeviceArray) that contains the data indices for the specified dimension.

```python
indices(dim : int)
```

- `dim`: The dimension for which to return the data indices.

### Properties
#### data
Gets a [DeviceArray](#DeviceArray) that contains the tensor data.

## DeviceArray
Represents a memory block of a specific [data type](#DataType) that is stored on a [device](#Device). Implements the Python Buffer protocol. Note that instances of `DeviceArray` can only be constructed in C++.

### Properties
#### size
Gets the size of the array.

#### dtype
Gets the [data type](#DataType) of the array.

#### device
Gets the [device](#Device) on which the array is stored.

## Device
Represents a particular data processing unit on the host system.

```python
Device(kind : DeviceKind, id : int)
```

- `kind`: The [kind of the device](#DeviceKind) (e.g. CPU, CUDA)
- `id`: The id of the device; i.e. if you have two CUDA-capable GPUs their IDs will be 0 and 1. For a device kind of CPU the ID will be always 0.

### Properties
#### kind
Gets the [kind of the device](#DeviceKind).

#### id
Gets the id of the device.

## DeviceKind
Represents a device kind that has data processing capabilities such as CPU or CUDA-capable GPU. Note that instances of `DeviceKind` can only be constructed in C++.

> As of today ML-IO does not have GPU support and the only available device kind is CPU.

### Properties
#### cpu (static)
Gets an instance for the CPU device kind.

#### name
Gets the name of the device kind.

## Enumerations
### DataType
Specifies the data type of a [Tensor](#Tensor) or a [DeviceArray](#DeviceArray).

| Value     | Description                                            |
|-----------|--------------------------------------------------------|
| `SIZE`    | A platform-specific unsigned integer type that can store the maximum size of a theoretically possible array (corresponds to `size_t` in C and C++). |
| `FLOAT16` | 16-bit floating-point number (a.k.a. half precision)   |
| `FLOAT32` | 32-bit floating-point number (a.k.a. single precision) |
| `FLOAT64` | 64-bit floating-point number (a.k.a. double precision) |
| `SINT8`   | Signed 8-bit integer                                   |
| `SINT16`  | Signed 16-bit integer                                  |
| `SINT32`  | Signed 32-bit integer                                  |
| `SINT64`  | Signed 64-bit integer                                  |
| `UINT8`   | Unsigned 8-bit integer                                 |
| `UINT16`  | Unsigned 16-bit integer                                |
| `UINT32`  | Unsigned 32-bit integer                                |
| `UINT64`  | Unsigned 64-bit integer                                |
| `STRING`  | A null-terminated string; in Python it is exposed as a `str` instance. |
