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

import pandas as pd

from mlio.integ.numpy import as_numpy


def to_pandas(example):
    """
    Converts the specified ``Example`` to a pandas DataFrame.
    """
    data = {attr.name: as_numpy(ftr).flatten()
            for attr, ftr in zip(example.schema.attributes, example)}

    return pd.DataFrame(data)
