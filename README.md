[![Download](https://img.shields.io/conda/pn/mlio/mlio-py)](https://anaconda.org/mlio/mlio-py)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue)](LICENSE)

# MLIO
MLIO is a high performance data access library for machine learning tasks with support for multiple data formats. It makes it easy for scientists to train models on their data without worrying about the format or where it's stored. Algorithm developers can also use MLIO to build production-quality algorithms that support a rich variety of data formats and provide helpful parsing and validation messages to their customers without compromising on performance.

MLIO is already being leveraged by various components of the [Amazon SageMaker](https://aws.amazon.com/sagemaker/) platform such as its first-party algorithms and the [Autopilot](https://aws.amazon.com/sagemaker/autopilot/) feature. The open-source Amazon SageMaker [XGBoost](https://github.com/aws/sagemaker-xgboost-container) and [Scikit-learn](https://github.com/aws/sagemaker-scikit-learn-container) container images also use MLIO for consuming datasets.

* [Installation](#Installation)
    * [Binaries](#Binaries)
    * [From Source](#From-Source)
* [Getting Started](#Getting-Started)
* [Reference](#Reference)
    * [Python](#Python)
    * [C++](#C++)
* [Support](#Support)
* [How to Contribute](#How-to-Contribute)

## Installation
> MLIO is only available for Linux and macOS operating systems.

### Binaries
#### Conda
The easiest way to get started with MLIO is installing it via [Conda](https://conda.io). The latest version is available in the *mlio* channel and can be installed into a Conda environment using the following commands:

```bash
$ conda create -name <env_name> python=3.7
$ conda activate <env_name>
$ conda install -c mlio -c conda-forge mlio-py
```

The *mlio* channel provides the following packages:

|Package                                                   |Description                                   |
|----------------------------------------------------------|----------------------------------------------|
| [mlio-py](https://anaconda.org/mlio/mlio-py)             | Language binding for Python 3.6 and later    |
| [libmlio](https://anaconda.org/mlio/libmlio)             | Runtime library                              |
| [libmlio-devel](https://anaconda.org/mlio/libmlio-devel) | Header files and other development artifacts |
| [libmlio-dbg](https://anaconda.org/mlio/libmlio-dbg)     | Debug symbols                                |

#### PyPI

MLIO is not available in PyPI due to technical challenges involved in supporting non-Python dependencies in pip environments. [See the blog post](https://uwekorn.com/2019/09/15/how-we-build-apache-arrows-manylinux-wheels.html) of Uwe L. Korn from the Apache Arrow project to learn more about the issues they faced with pip and why they dropped their official support for it. These are the very same issues our team has faced and the reason why do not support pip/wheel at the moment.

### From Source
> Unless you want to contribute to MLIO we strongly recommend installing it in binary form as described above. Building MLIO from scratch involves setting up a specific development environment and can be a tedious task if you are not comfortable with various tools and technologies (e.g. toolchains, CMake, C++).

Instructions on how to build MLIO locally can be found [here](doc/build.md).

## Getting Started
MLIO currently supports reading three data formats: CSV, Parquet, and RecordIO-Protobuf.

Datasets read with MLIO can be converted into NumPy arrays, SciPy COO matrices, pandas DataFrames, TensorFlow tensors, PyTorch tensors, and MXNet arrays. Below we show some examples on how to read and convert data with MLIO.

### Reading CSV Files as NumPy Arrays

One or more CSV files can be read as a single dataset using the [`CsvReader`](doc/python/data_reader.md#CsvReader) class. The code snippet below shows how you can quickly iterate through a CSV dataset in mini-batches.

```python
import numpy as np
import mlio

from mlio.integ.numpy import as_numpy

file_dir = '/path/to/csv_data' # This can be a directory or a single file.
dataset = mlio.list_files(file_dir, pattern='*.csv')

# CsvReader supports an extensive set of constructor parameters. Here we
# just specify the two required arguments.
reader_params = mlio.DataReaderParams(dataset=dataset, batch_size=200)
reader = mlio.CsvReader(reader_params)

num_epochs = 5 # Number of times to read the full dataset.
for epoch in range(num_epochs):
    # CsvReader is simply an iterator over mini-batches of data.
    for example in reader:
        # An ``Example`` instance acts like a dictionary of MLIO tensors
        # mapped by column name according to the CSV header.
        lbl = example['label'] # Get the MLIO ``Tensor`` of the column called 'label'.
        lbl = as_numpy(lbl) # Zero-copy convert the tensor into a NumPy array.

        # Alternatively, transform the mini-batch into a NumPy array.
        batch = np.column_stack([as_numpy(feature) for feature in example])
        ...
    reader.reset() # Return to the beginning of dataset.
 ```

### Reading CSV Files over Amazon SageMaker Pipe Mode as pandas DataFrames
MLIO can read datasets from various sources including local file system, in-memory buffers, and Amazon SageMaker Pipe mode. Below we show how you can read a CSV dataset over Amazon SageMaker Pipe mode.

```python
import numpy as np
import mlio

from mlio.integ.pandas import to_pandas

# Instead of getting a list of ``mlio.File`` instances via ``mlio.list_files()``,
# this time we construct an ``mlio.SageMakerPipe`` instance. 
pipe = mlio.SageMakerPipe('/opt/ml/train')

reader_params = mlio.DataReaderParams(dataset=[pipe], batch_size=200)
reader = mlio.CsvReader(reader_params)

num_epochs = 5 # Number of times to read the full dataset.
for epoch in range(num_epochs):
    # CsvReader is simply an iterator over mini-batches of data.
    for example in reader:
        # Convert the mini-batch into a pandas DataFrame
        df = to_pandas(example)
        ...
    reader.reset() # Return to the beginning of dataset.
```

### Reading Images as NumPy Arrays

MLIO supports reading image files in JPEG and PNG formats, using the [`ImageReader`](doc/python/data_reader.md#ImageReader) class. The code snippet below shows how you can quickly iterate through an image dataset in mini-batches.

```python
import numpy as np
import mlio

from mlio.integ.numpy import as_numpy

file_dir = '/path/to/image_dir' # This can be a directory or a single file.

# JPEG or PNG images formats are supported.
dataset = mlio.list_files(file_dir, pattern='*.jpg')

data_reader_params = mlio.DataReaderParams(dataset=dataset, batch_size=200)

# Setting ImageReader parameters.
image_reader_prm = mlio.ImageReaderParams(image_frame=mlio.ImageFrame.NONE,
                                          resize=500,
                                          image_dimensions=[3,166,190],
                                          to_rgb=1)

reader = mlio.ImageReader(data_reader_params, image_reader_prm)

num_epochs = 5 # Number of times to read the full dataset.
for epoch in range(num_epochs):
    # ImageReader is simply an iterator over mini-batches of data.
    for example in reader:
        # The 'value' tensor will contain the batch of images in NHWC
        # (batch * height * width * channel) format.
        lbl = example['value']
        # Zero-copy convert the tensor into a NumPy array.
        lbl = as_numpy(lbl)
        ...
    reader.reset() # Return to the beginning of dataset.
 ```

### Reading RecordIO-protobuf Files over Amazon S3 as PyTorch Tensors
[RecordIO-protobuf](https://docs.aws.amazon.com/sagemaker/latest/dg/cdf-training.html) is the native data format of first-party Amazon SageMaker algorithms. It is a binary format that is specifically tuned for high-throughput. With MLIO third-party algorithms can now leverage the same performance benefit as first-party Amazon SageMaker algorithms.

```python
import numpy as np
import mlio

from mlio.integ.torch import as_torch

# Make sure that AWS C++ SDK is initialized.
mlio.initialize_aws_sdk()

# Use default AWS credentials.
client = mlio.S3Client()

dataset = mlio.list_s3_objects(client, 's3://path/to/recordio_data')

# RecordIOProtobufReader supports an extensive set of constructor
# parameters. Here we just specify the two required arguments.
reader_params = mlio.DataReaderParams(dataset=dataset, batch_size=200)
reader = mlio.RecordIOProtobufReader(reader_params)

num_epochs = 5  # Number of times to read the full dataset.
for epoch in range(num_epochs):
    # RecordIOProtobufReader is simply an iterator over mini-batches of data.
    for example in reader:
        # For 1p Amazon SageMaker algorithms, Recordio-protobuf is
        # expected to have two features: 'label_values', 'values'
        lbl_tensor = example['label_values']
        val_tensor = example['values']

        # Zero-copy convert the features to PyTorch tensors.
        lbl_tensor = as_torch(lbl_tensor)
        val_tensor = as_torch(val_tensor)

        ...
    reader.reset()
    
 # Optionally deallocate internal data structures used by AWS C++ SDK (recommended).
 mlio.deallocate_aws_sdk()
 ```

### Reading Parquet Files over Amazon SageMaker Pipe Mode with Apache Arrow
MLIO offers native integration with Apache Arrow and can represent dataset records as Arrow files. In the example below we read a Parquet dataset from an Amazon SageMaker Pipe channel and pass it to Arrow for actual parsing.

```python
import pyarrow.parquet as pq
import mlio

from mlio.integ.arrow import as_arrow_file

# Get a handle to the SageMaker Pipe channel that streams a Parquet
# dataset.
pipe = mlio.SageMakerPipe('/opt/ml/input/data/train')

with pipe.open_read() as strm:
    # Construct an ``mlio.ParquetRecordReader`` that extracts each
    # Parquet file from the pipe stream.
    reader = mlio.ParquetRecordReader(strm)

    for record in reader:
        # Wrap each record (Parquet file) as an Arrow file and read into
        # an Arrow table.
        table = pq.read_table(as_arrow_file(record))
        ...
```

### Reading in C++
The C++ API of MLIO has full feature parity with its Python API. In fact MLIO is mostly written in modern C++ and exposes its functionality to Python using a thin language binding layer. This makes it possible to perform quick experimentations and fast iterations in Python that can later be productionized in C++ with very little effort.

Below we show the same [`CsvReader`](doc/python/data_reader.md#CsvReader) sample code; this time exporting columns as DLPack tensors instead of NumPy arrays.

```cpp
#include <mlio.h>

int main(int argc, char *argv[])
{
    // Initialize the library.
    mlio::initialize();

    auto dataset = mlio::list_files("/path/to/csv_data", /*pattern=*/"*.csv");

    // csv_reader supports an extensive set of constructor parameters.
    // Here we just specify the two required arguments.
    mlio::data_reader_params prm{dataset, /*batch_size=*/200};

    auto reader = mlio::make_intrusive<mlio::csv_reader>(prm);

    // Read the dataset five times (five epochs).
    for (auto i = 0; i < /*num_epochs*/ 5; i++) {
        // An example instance acts like a dictionary of MLIO tensors
        // mapped by column name according to the CSV header.
        mlio::intrusive_ptr<mlio::example> exm;

        // csv_reader is simply an iterator over mini-batches of data.
        while ((exm = reader->read_example()) != nullptr) {
            // Get the MLIO tensor of the column called 'label'.
            auto lbl = exm->find_feature("label");

            // Zero-copy convert it to DLPack.
            DLManagedTensor *dl = mlio::as_dlpack(*lbl);

            // Share the DLPack with other frameworks (i.e. Torch, MXNet)
            ...
        }

        reader->reset();
    }
}
```

## Reference

MLIO uses a layered architecture as shown in the following figure. Check out the Python and C++ reference links below to learn more about MLIO's API.

<p align="center"><image src="doc/architecture.png"></p>

### Python
* [Data Stores](doc/python/data_store.md)
* [Streams](doc/python/stream.md)
* [Record Readers](doc/python/record_reader.md)
* [Data Readers](doc/python/data_reader.md)
* [Tensors](doc/python/tensor.md)
* [Framework/Library Integration](doc/python/integration.md)
* [Logging](doc/python/logging.md)
* [Miscellaneous](doc/python/misc.md)
* [Contrib Extensions](src/mlio-py/mlio/contrib)

### C++
* TBD

## Support
Please submit your questions, feature requests, and bug reports on [GitHub issues](https://github.com/awslabs/ml-io/issues) page.

## How to Contribute
We welcome community contributions to MLIO. Please read our [Contributing Guidelines](CONTRIBUTING.md) to learn more.

## License
This project is licensed under the Apache-2.0 License.
