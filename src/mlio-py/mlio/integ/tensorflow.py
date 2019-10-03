# Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"). You
# may not use this file except in compliance with the License. A copy of
# the License is located at
#
#      http://aws.amazon.com/apache2.0/
#
# or in the "license" file accompanying this file. This file is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
# ANY KIND, either express or implied. See the License for the specific
# language governing permissions and limitations under the License.

import numpy as np
import tensorflow as tf

from mlio.integ.numpy import as_numpy
from mlio.integ.scipy import to_coo_matrix


def as_tensorflow(tensor):
    return tf.convert_to_tensor(as_numpy(tensor))


def as_tensorflow_sparse(tensor):
    mtx = to_coo_matrix(tensor).tocsr()

    non_zero_row_col = mtx.nonzero()
    indices = np.asmatrix([non_zero_row_col[0], non_zero_row_col[1]])
    indices = indices.transpose()

    return tf.SparseTensor(indices, mtx.data, mtx.shape)


def make_tensorflow_dataset(data_reader, features, dtypes):
    def generator():
        for exm in data_reader:
            feature_tensor_dict = {}
            for feature in features:
                feature_tensor_dict[feature] = exm[feature]
            yield feature_tensor_dict

    output_types = {k: l for (k, l) in zip(features, dtypes)}
    return tf.data.Dataset.from_generator(generator, output_types)
