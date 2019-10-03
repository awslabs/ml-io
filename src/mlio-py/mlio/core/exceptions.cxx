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

#include "core/module.h"

namespace py = pybind11;

namespace mliopy {

void
register_exceptions(py::module &m)
{
    PyObject *base;

    base = py::register_exception<mlio::corrupt_record_error>(
        m, "CorruptRecordError", ::PyExc_RuntimeError).ptr();
    py::register_exception<mlio::corrupt_footer_error>(
        m, "CorruptFooterError", base);
    py::register_exception<mlio::corrupt_header_error>(
        m, "CorruptHeaderError", base);

    base = py::register_exception<mlio::stream_error>(
        m, "StreamError", ::PyExc_RuntimeError).ptr();
    py::register_exception<mlio::inflate_error>(
        m, "InflateError", base);

    base = py::register_exception<mlio::data_reader_error>(
        m, "DataReaderError", ::PyExc_RuntimeError).ptr();
    py::register_exception<mlio::schema_error>(
        m, "SchemaError", base);
    py::register_exception<mlio::invalid_instance_error>(
        m, "InvalidInstanceError", base);

    py::register_exception<mlio::not_supported_error>(
        m, "NotSupportedError", ::PyExc_RuntimeError);
}

}  // namespace mliopy
