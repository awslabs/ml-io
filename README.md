# ML-IO

ML-IO is a high performance data access library for machine learning tasks with support for multiple data formats. It makes it easy for scientists to train models on their data without worrying about the format or where it's stored. Algorithm developers can also use ML-IO to build production-quality algorithms that support a rich variety of data formats and provide helpful parsing and validation messages to their customers without compromising on performance.

* [Installation](#installation)
    * [Binaries](#binaries)
    * [From Source](#from-source)
* [Getting Started](#getting-started)
    * [CsvReader](#csvreader)
    * [RecordIOProtobufReader](#recordioprotobufreader)
    * [ParquetRecordReader](#parquetrecordreader)
* [Support](#support)
* [How to Contribute](#how-to-contribute)
* [License](#license)

## Installation

### Binaries

Conda is required for ML-IO. Instructions for installing Anaconda/Miniconda can be found [here](https://docs.conda.io/projects/conda/en/latest/user-guide/install/). The follow is an example for installing Miniconda3 in Linux.
```
curl -LO http://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh
bash Miniconda3-latest-Linux-x86_64.sh -bfp /miniconda3
rm Miniconda3-latest-Linux-x86_64.sh
export PATH=/miniconda3/bin:${PATH}
conda update -y conda
```

Optionally install pyarrow to read parquet.
```
conda install -c conda-forge pyarrow=0.14.1
```

Finally, install ML-IO.
```
conda install -c mlio -c conda-forge mlio-py
```

### From Source

TODO

## Getting Started

ML-IO currently supports reading 3 data formats: CSV, RecordIO-Protobuf, and Parquet.

### CsvReader

The CsvReader class is a DataReader object which reads blocks of data from a source. Here is an example of its use when reading from a file:
```
import numpy as np
import mlio
from mlio.integ.numpy import as_numpy

file_dir = '/home/user/csv_data' # This can also be a single csv file
dataset = mlio.list_files(file_dir, pattern='*.csv') # List of file(s)
reader = mlio.CsvReader(dataset=dataset,
                        batch_size=200)

num_epochs = 5 # Number of times to read the full dataset
for epoch in range(num_epochs):
    # The CsvReader is an iterator over batches of data
    for example in reader:
        # An example acts like a dictionary of ml-io tensors mapped by
        # feature name according to the csv header
        tensor_a = example['a'] # Getting the tensor of a single feature called 'a'
        tensor_a = as_numpy(tensor_a) # Convert the tensor into a numpy array

        # Alternatively, transform the whole example into a numpy array
        tensors = [as_numpy(feature).squeeze() for feature in example]
        batch = np.vstack(tensors) # Stack tensors vertically
        # Each tensor is 1-dimensional, so the result of the stack has a transposed shape
        batch = batch.T
        ...
    reader.reset() # Return to top of dataset

# If your file does not have a header, specify the header_row_index param as None
# Now the feature names are '1','2', ... 'n'
reader = mlio.CsvReader(dataset=dataset,
                        batch_size=200,
                        header_row_index=None)
```

You can also read data from a SageMaker pipe stream:
```
import numpy as np
import mlio
from mlio.integ.numpy import as_numpy

pipe_name = '/opt/ml/train' # Pipe name excluding the epoch
dataset = mlio.SageMakerPipe(pipe_name)
reader = mlio.CsvReader(dataset=[dataset],
                        batch_size=200)

# No difference in reading examples
# reader.reset() closes the current pipe and advances to the next one
```

### RecordIOProtobufReader

The RecordIOProtobufReader class is also a DataReader which reads blocks of data from a source. Its use is very similar to the CsvReader:
```
import numpy as np
import mlio
from mlio.integ.numpy import as_numpy

file_dir = '/home/user/recordio_data'
dataset = mlio.list_files(file_dir)
reader = mlio.RecordIOProtobufReader(dataset=dataset,
                                     batch_size=200)

num_epochs = 5
for epoch in range(num_epochs):
    for example in reader:
        # For SageMaker, recordio-protobuf is expected to have two features (in order):
        # 'label_values', 'values'
        label_tensor = example['label_values']
        label_tensor = as_numpy(label_tensor)
        
        data_tensor = example['values']
        data_tensor = as_numpy(data_tensor)
        
        # Alternatively, transform the whole example into a numpy array
        tensors = [as_numpy(feature) for feature in example]
        batch = np.hstack(tensors)
        # These tensors are 2-dimensional and only need to be stacked horizontally
        ...
    reader.reset()
 ```

Similarly, to read data from a SageMaker pipe stream:
```
import numpy as np
import mlio
from mlio.integ.numpy import as_numpy

pipe_name = '/opt/ml/train'
dataset = mlio.SageMakerPipe(pipe_name)
reader = mlio.RecordIOProtobufReader(dataset=dataset,
                                     batch_size=200)

# No difference in reading examples
# reader.reset() closes the current pipe and advances to the next one
```

### ParquetRecordReader

The ParquetRecordReader is not a DataReader object, but rather a RecordReader. It reads into memory one parquet file at a time from an input stream which can then be fed into a pyarrow Table:
```
import mlio
from mlio.integ.arrow import as_arrow_file

import pyarrow.parquet as pq

# Single file
file = mlio.File('/home/user/file.parquet')

num_epochs = 5
for epoch in range(num_epochs):
    with file.open_read() as strm:
        reader = mlio.ParquetRecordReader(strm)
        
        for record in reader:
            table = pq.read_table(as_arrow_file(record))
            ...
        
# Dataset
file_dir = '/home/user/parquet_data'
dataset = mlio.list_files(file_dir)

num_epochs = 5
for epoch in range(num_epochs):
    for file in dataset:
        with file.open_read() as strm:
            reader = mlio.ParquetRecordReader(strm)
            
            for record in reader:
                table = pq.read_table(as_arrow_file(record))
                ...
```

However, pyarrow has [native methods](https://arrow.apache.org/docs/python/parquet.html) for reading parquet data from a file/directory of files, for example:
```
import pyarrow.parquet as pq

table = pq.read_table('/home/user/file.parquet') # Single file

table = pq.read_table('home/user/parquet_data') # Dataset
```

The primary benefit of the ParquetRecordReader is in reading data from a SageMaker pipe stream:
```
import mlio
from mlio.integ.arrow import as_arrow_file

import pyarrow.parquet as pq

pipe = mlio.SageMakerPipe('/opt/ml/train')

num_epochs = 5
for epoch in range(num_epochs):
    with pipe.open_read() as strm:
        reader = mlio.ParquetRecordReader(strm)
    
        for record in reader:
            table = pq.read_table(as_arrow_file(record))
            ...
```

## Support

Please submit your questions, feature requests, and bug reports on [GitHub issues](https://github.com/awslabs/ml-io/issues) page.

## How to Contribute

We welcome community contributions to ML-IO. Please read our [Contributing Guidelines](CONTRIBUTING.md) to learn more.

## License

This project is licensed under the Apache-2.0 License.
