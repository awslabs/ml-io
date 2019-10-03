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

from mlio.core import CooTensor
from scipy.sparse import coo_matrix


def to_coo_matrix(tensor):
    """
    Converts the specified tensor to a ``coo_matrix``.
    """

    if not isinstance(tensor, CooTensor):
        raise ValueError("The tensor must be an instance of CooTensor.")

    s = tensor.shape

    if len(s) > 2:
        raise ValueError("Only one- and two-dimensional COO tensors are "
                         "supported.")

    if len(s) == 1:
        s = (1,) + s

    data = np.array(tensor.data, copy=False)
    rows = np.array(tensor.indices(0), copy=False)
    cols = np.array(tensor.indices(1), copy=False)

    return coo_matrix((data, (rows, cols)), s, copy=True)


def to_tensor(mtx):
    """
    Converts the specified ``coo_matrix`` to a tensor.
    """

    if not isinstance(mtx, coo_matrix):
        raise ValueError("Only coo_matrix is supported.")

    rows = mtx.row
    cols = mtx.col

    rows = rows.astype(np.int64, copy=True)
    cols = cols.astype(np.int64, copy=True)

    return CooTensor(mtx.shape, mtx.data, [rows, cols], copy=False)
