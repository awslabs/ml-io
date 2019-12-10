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

#include "module.h"

#include <mlio/record_readers/parquet_record_reader.h>

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

class py_record_iterator {
public:
    explicit py_record_iterator(record_reader &rdr, py::object parent)
        : reader_{&rdr}, parent_{std::move(parent)}
    {}

public:
    std::optional<record>
    next()
    {
        std::optional<record> rec = reader_->read_record();
        if (rec == std::nullopt) {
            throw py::stop_iteration();
        }

        return rec;
    }

private:
    record_reader *reader_;
    py::object parent_;
};

}  // namespace

void
register_record_readers(py::module &m)
{
    py::enum_<record_kind>(m, "RecordKind")
        .value("COMPLETE", record_kind::complete)
        .value("BEGIN", record_kind::begin)
        .value("MIDDLE", record_kind::middle)
        .value("END", record_kind::end);

    py::class_<record>(m,
                       "Record",
                       py::buffer_protocol(),
                       "Represents an encoded record read from a data store.")
        .def_property_readonly("kind", &record::kind)
        .def_buffer([](record &r) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            auto *data = const_cast<std::byte *>(r.payload().data());

            auto size = static_cast<py::ssize_t>(r.payload().size());

            // TODO(balioglu): Make read-only.
            return py::buffer_info(data, 1, "B", size);
        });

    py::class_<py_record_iterator>(m, "RecordIterator")
        .def("__iter__",
             [](py_record_iterator &it) -> py_record_iterator & {
                 return it;
             })
        .def("__next__", &py_record_iterator::next);

    py::class_<record_reader, intrusive_ptr<record_reader>>(m, "RecordReader")
        .def("read_record",
             &record_reader::read_record,
             py::call_guard<py::gil_scoped_release>())
        .def("peek_record",
             &record_reader::peek_record,
             py::call_guard<py::gil_scoped_release>())
        .def("__iter__", [](py::object &rdr) {
            return py_record_iterator(rdr.cast<record_reader &>(), rdr);
        });

    py::class_<mlio::detail::parquet_record_reader,
               record_reader,
               intrusive_ptr<mlio::detail::parquet_record_reader>>(
        m,
        "ParquetRecordReader",
        "Represents a ``record_reader`` for reading Parquet records.")
        .def(py::init<intrusive_ptr<input_stream>>(), "strm"_a);
}

}  // namespace pymlio
