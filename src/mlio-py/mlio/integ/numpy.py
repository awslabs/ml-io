# Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

from mlio._core import DenseTensor


def as_numpy(tensor):
    """
    Wraps the specified Tensor as a NumPy array.
    """
    if not isinstance(tensor, DenseTensor):
        raise ValueError("Only dense tensors can be converted to a NumPy "
                         "array.")

    return np.array(tensor, copy=False)


def to_numpy(tensor):
    """
    Converts the specified Tensor to a NumPy array.
    """
    if not isinstance(tensor, DenseTensor):
        raise ValueError("Only dense tensors can be converted to a NumPy "
                         "array.")

    return np.array(tensor)


def as_tensor(arr):
    """
    Wraps the specified NumPy array as a Tensor.
    """
    if not np.issctype(arr.dtype):
        raise ValueError("A non-scalar array cannot be converted to a Tensor "
                         "without copying.")

    return DenseTensor(arr.shape,
                       data=arr,
                       strides=tuple(s // 8 for s in arr.strides),
                       copy=False)


def to_tensor(arr):
    """
    Converts the specified NumPy array to a Tensor.
    """
    return DenseTensor(arr.shape,
                       data=arr,
                       strides=tuple(s // 8 for s in arr.strides))
