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

#include <mlio/record_readers/parquet_record_reader.h>

namespace py = pybind11;
namespace stdx = mlio::stdx;

using namespace pybind11::literals;

namespace mliopy {
namespace detail {
namespace {

class py_record_iterator {
public:
    explicit
    py_record_iterator(mlio::record_reader &rdr, py::object parent)
        : reader_{&rdr}, parent_{std::move(parent)}
    {}

public:
    stdx::optional<mlio::record>
    next()
    {
        stdx::optional<mlio::record> rec = reader_->read_record();
        if (rec == stdx::nullopt) {
            throw py::stop_iteration();
        }

        return rec;
    }

private:
    mlio::record_reader *reader_;
    py::object parent_;
};

}  // namespace
}  // namespace detail

void
register_record_readers(py::module &m)
{
    py::enum_<mlio::record_kind>(m, "RecordKind")
        .value("COMPLETE", mlio::record_kind::complete)
        .value("BEGIN",    mlio::record_kind::begin)
        .value("MIDDLE",   mlio::record_kind::middle)
        .value("END",      mlio::record_kind::end);

    py::class_<mlio::record>(m, "Record", py::buffer_protocol(),
        "Represents an encoded record read from a dataset.")
        .def_property_readonly("kind", &mlio::record::kind)
        .def_buffer([](mlio::record &r)
            {
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                auto *data = const_cast<stdx::byte *>(r.payload().data());

                auto size = static_cast<py::ssize_t>(r.payload().size());

                // TODO(balioglu): Make read-only.
                return py::buffer_info(data, 1, "B", size);
            });

    py::class_<detail::py_record_iterator>(m, "RecordIterator")
        .def("__iter__", [](detail::py_record_iterator &it) -> detail::py_record_iterator &
            {
                return it;
            })
        .def("__next__", &detail::py_record_iterator::next);

    py::class_<mlio::record_reader,
               mlio::intrusive_ptr<mlio::record_reader>>(m, "RecordReader")
        .def("read_record", &mlio::record_reader::read_record,
            py::call_guard<py::gil_scoped_release>())
        .def("peek_record", &mlio::record_reader::peek_record,
            py::call_guard<py::gil_scoped_release>())
        .def("__iter__", [](py::object &rdr)
            {
                return detail::py_record_iterator(rdr.cast<mlio::record_reader &>(),
                                                  rdr);
            });

    py::class_<mlio::detail::parquet_record_reader,
               mlio::record_reader,
               mlio::intrusive_ptr<mlio::detail::parquet_record_reader>>(m, "ParquetRecordReader",
        "Represents a ``record_reader`` for reading Parquet records.")
        .def(py::init<mlio::intrusive_ptr<mlio::input_stream>>(), "strm"_a);
}

}  // namespace mliopy
