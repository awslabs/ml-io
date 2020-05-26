/*
 * Copyright 2019-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

class Py_record_iterator {
public:
    explicit Py_record_iterator(Record_reader &reader, py::object parent)
        : reader_{&reader}, parent_{std::move(parent)}
    {}

public:
    std::optional<Record> next()
    {
        std::optional<Record> record{};
        {
            py::gil_scoped_release rel_gil;

            record = reader_->read_record();
        }

        if (record == std::nullopt) {
            throw py::stop_iteration();
        }

        return record;
    }

private:
    Record_reader *reader_;
    py::object parent_;
};

}  // namespace

void register_record_readers(py::module &m)
{
    py::enum_<Record_kind>(m, "RecordKind")
        .value("COMPLETE", Record_kind::complete)
        .value("BEGIN", Record_kind::begin)
        .value("MIDDLE", Record_kind::middle)
        .value("END", Record_kind::end);

    py::class_<Record>(
        m, "Record", py::buffer_protocol(), "Represents an encoded Record read from a data store.")
        .def_property_readonly("kind", &Record::kind)
        .def_buffer([](Record &r) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            auto *data = const_cast<std::byte *>(r.payload().data());

            auto size = static_cast<py::ssize_t>(r.payload().size());

            // TODO(balioglu): Make read-only.
            return py::buffer_info(data, 1, "B", size);
        });

    py::class_<Py_record_iterator>(m, "RecordIterator")
        .def("__iter__",
             [](Py_record_iterator &it) -> Py_record_iterator & {
                 return it;
             })
        .def("__next__", &Py_record_iterator::next);

    py::class_<Record_reader, Intrusive_ptr<Record_reader>>(m, "RecordReader")
        .def("read_record", &Record_reader::read_record, py::call_guard<py::gil_scoped_release>())
        .def("peek_record", &Record_reader::peek_record, py::call_guard<py::gil_scoped_release>())
        .def("__iter__", [](py::object &reader) {
            return Py_record_iterator(reader.cast<Record_reader &>(), reader);
        });

    py::class_<mlio::detail::Parquet_record_reader,
               Record_reader,
               Intrusive_ptr<mlio::detail::Parquet_record_reader>>(
        m, "ParquetRecordReader", "Represents a ``Record_reader`` for reading Parquet records.")
        .def(py::init<Intrusive_ptr<Input_stream>>(), "stream"_a);
}

}  // namespace pymlio
