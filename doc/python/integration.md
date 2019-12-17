# Framework/Library Integration
* [NumPy](#NumPy)
* [SciPy](#SciPy)
* [pandas](#pandas)
* [PyTorch](#PyTorch)
* [TensorFlow](#TensorFlow)
* [MXNet](#MXNet)
* [DLPack](#DLPack)

## NumPy
### as_numpy
Wraps the specified [`DenseTensor`](tensor.md#DenseTensor) as a NumPy array. This is a zero-copy operation; at the end both the tensor and the NumPy array will share the same data.

```python
mlio.integ.numpy.as_numpy(tensor : DenseTensor)
```

- `tensor`: A [`DenseTensor`](tensor.md#DenseTensor) instance.

### to_numpy
Copies the specified [`DenseTensor`](tensor.md#DenseTensor) as a NumPy array.

```python
mlio.integ.numpy.to_numpy(tensor : DenseTensor)
```

- `tensor`:  A [`DenseTensor`](tensor.md#DenseTensor) instance.

### as_tensor
Wraps the specified NumPy array as a [`DenseTensor`](tensor.md#DenseTensor). This is a zero-copy operation; at the end both the tensor and the NumPy array will share the same data.

```python
mlio.integ.numpy.as_tensor(arr : ndarray)
```

- `arr`: A NumPy array.

### to_tensor
Copies the specified NumPy array as a [`DenseTensor`](tensor.md#DenseTensor).

```python
mlio.integ.numpy.to_tensor(arr : ndarray)
```

- `arr`: A NumPy array.

## SciPy
### to_coo_matrix
Copies the specified [`CooTensor`](tensor.md#CooTensor) as a SciPy [`coo_matrix`](https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.coo_matrix.html). Note that `coo_matrix` supports only up to two dimensions. If your [`CooTensor`](tensor.md#CooTensor) instance has more than two dimensions, the function will raise an error.

```python
mlio.integ.scipy.to_coo_matrix(tensor : CooTensor)
```

- `tensor`: A [`CooTensor`](tensor.md#CooTensor) instance.

### to_tensor
Copies the specified SciPy [`coo_matrix`](https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.coo_matrix.html) as a [`CooTensor`](tensor.md#CooTensor).

```python
mlio.integ.scipy.to_tensor(mtx : coo_matrix)
```

- `mtx`: A [`coo_matrix`](https://docs.scipy.org/doc/scipy/reference/generated/scipy.sparse.coo_matrix.html) instance.

## pandas
### to_pandas
Copies the specified [`Example`](data_reader.md#Example) to a pandas [DataFrame](https://pandas.pydata.org/pandas-docs/stable/reference/api/pandas.DataFrame.html).

```python
mlio.integ.pandas.to_pandas(exm : Example)
```

- `exm`: The [`Example`](data_reader.md#Example) instance.

## PyTorch
### as_torch
Wraps the specified [`DenseTensor`](tensor.md#DenseTensor) as a Torch tensor. This is a zero-copy operation; at the end both tensors will share the same data.

```python
mlio.integ.torch.as_torch(tensor : DenseTensor)
```

- `tensor`: A [`DenseTensor`](tensor.md#DenseTensor) instance.

## TensorFlow
### to_tf
Copies the specified [`Tensor`](tensor.md#Tensor) to a dense or sparse TensorFlow tensor.

```python
mlio.integ.tensorflow.to_tf(tensor : Tensor)
```

- `tensor`: A [`Tensor`](tensor.md#Tensor) instance.

### make_tf_dataset
Constructs a TensorFlow dataset from the specified [`DataReader`](data_reader.md#DataReader) instance.

```python
mlio.integ.tensorflow.make_tf_dataset(data_reader : DataReader,
                                      features : Sequence[str],
                                      dtypes : Sequece[tf.DType])
```

- `data_reader`: The [`DataReader`](data_reader.md#DataReader) instance to wrap.
- `features`: The list of feature names.
- `dtypes`: The data types of features.

## MXNet
### as_mxnet
Wraps the specified [`DenseTensor`](tensor.md#DenseTensor) as an MXNet array. This is a zero-copy operation; at the end both the tensor and the MXNet array will share the same data.

```python
mlio.integ.mxnet.as_mxnet(tensor : DenseTensor)
```

- `tensor`: A [`DenseTensor`](tensor.md#DenseTensor) instance.

## DLPack
### as_dlpack
Wraps the specified [`DenseTensor`](tensor.md#DenseTensor) as a DLPack tensor returned in a Python [capsule](https://docs.python.org/3.6/c-api/capsule.html).  This is a zero-copy operation; at the end both tensors will share the same data.

```python
mlio.integ.dlpack.as_dlpack(tensor : DenseTensor, version : int = 0x10)
```

- `tensor`: A [`DenseTensor`](tensor.md#DenseTensor) instance.
- `version`: The DLPack specification version that the tensor should be compatible with.
