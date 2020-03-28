/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"). You
 * may not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 *      http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
 * ANY KIND, either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#pragma once

#include <pybind11/chrono.h>
#include <pybind11/functional.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <vector>

#include <mlio.h>

PYBIND11_DECLARE_HOLDER_TYPE(T, mlio::intrusive_ptr<T>, true);

PYBIND11_MAKE_OPAQUE(std::vector<mlio::feature_desc>);

namespace pymlio {

void
register_exceptions(pybind11::module &m);

void
register_logging(pybind11::module &m);

void
register_s3_client(pybind11::module &m);

void
register_memory_slice(pybind11::module &m);

void
register_device_array(pybind11::module &m);

void
register_tensors(pybind11::module &m);

void
register_integ(pybind11::module &m);

void
register_streams(pybind11::module &m);

void
register_data_stores(pybind11::module &m);

void
register_record_readers(pybind11::module &m);

void
register_schema(pybind11::module &m);

void
register_example(pybind11::module &m);

void
register_data_readers(pybind11::module &m);

}  // namespace pymlio
