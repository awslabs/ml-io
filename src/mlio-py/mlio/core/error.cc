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

#include <exception>
#include <system_error>
#include <unordered_map>

namespace py = pybind11;

using namespace mlio;

namespace pymlio {
namespace {

std::unordered_map<std::string, py::object> py_exc_types{};

template<typename T>
inline PyObject *register_exception(py::module &m, const char *name, PyObject *base)
{
    auto r = py_exc_types.emplace(name, py::exception<T>{m, name, base});

    return r.first->second.ptr();
}

void throw_nested_error(const std::exception &exc, const py::object &py_exc);

void set_nested_error(const py::object &outer_py_exc,
                      const std::exception &nested_exc,
                      const char *name)
{
    py::object nested_py_exc = py_exc_types[name](nested_exc.what());

    throw_nested_error(nested_exc, nested_py_exc);

    ::PyException_SetCause(outer_py_exc.ptr(), nested_py_exc.release().ptr());
}

void set_nested_system_error(const py::object &outer_py_exc, const std::system_error &nested_exc)
{
    py::object nested_py_exc =
        py::handle{::PyExc_OSError}(nested_exc.code().value(), nested_exc.what());

    throw_nested_error(nested_exc, nested_py_exc);

    ::PyException_SetCause(outer_py_exc.ptr(), nested_py_exc.release().ptr());
}

void throw_nested_error(const std::exception &exc, const py::object &py_exc)
{
    try {
        std::rethrow_if_nested(exc);
    }
    catch (const Not_supported_error &e) {
        set_nested_error(py_exc, e, "NotSupportedError");
    }
    catch (const Invalid_instance_error &e) {
        set_nested_error(py_exc, e, "InvalidInstanceError");
    }
    catch (const Schema_error &e) {
        set_nested_error(py_exc, e, "SchemaError");
    }
    catch (const Data_reader_error &e) {
        set_nested_error(py_exc, e, "DataReaderError");
    }
    catch (const Record_too_large_error &e) {
        set_nested_error(py_exc, e, "RecordTooLargeError");
    }
    catch (const Corrupt_footer_error &e) {
        set_nested_error(py_exc, e, "CorruptFooterError");
    }
    catch (const Corrupt_header_error &e) {
        set_nested_error(py_exc, e, "CorruptHeaderError");
    }
    catch (const Corrupt_record_error &e) {
        set_nested_error(py_exc, e, "CorruptRecordError");
    }
    catch (const Record_error &e) {
        set_nested_error(py_exc, e, "RecordError");
    }
    catch (const Inflate_error &e) {
        set_nested_error(py_exc, e, "InflateError");
    }
    catch (const Stream_error &e) {
        set_nested_error(py_exc, e, "StreamError");
    }
    catch (const Mlio_error &e) {
        set_nested_error(py_exc, e, "MLIOError");
    }
    catch (const std::system_error &e) {
        set_nested_system_error(py_exc, e);
    }
}

void set_error(const std::exception &exc, const char *name)
{
    py::handle &py_exc_type = py_exc_types[name];

    py::object py_exc = py_exc_type(exc.what());

    throw_nested_error(exc, py_exc);

    ::PyErr_SetObject(py_exc_type.ptr(), py_exc.ptr());
}

void set_system_error(const std::system_error &exc)
{
    py::object py_exc = py::handle{::PyExc_OSError}(exc.code().value(), exc.what());

    throw_nested_error(exc, py_exc);

    ::PyErr_SetObject(::PyExc_OSError, py_exc.ptr());
}

}  // namespace

void register_exceptions(py::module &m)
{
    // clang-format off

    PyObject *py_mlio_error = register_exception<Mlio_error>(
        m, "MLIOError", ::PyExc_RuntimeError);

    PyObject *py_stream_error = register_exception<Stream_error>(
        m, "StreamError", py_mlio_error);
    register_exception<Inflate_error>(
        m, "InflateError", py_stream_error);

    PyObject *py_record_error = register_exception<Record_error>(
        m, "RecordError", py_mlio_error);

    PyObject *py_corrupt_record_error = register_exception<Corrupt_record_error>(
        m, "CorruptRecordError", py_record_error);
    register_exception<Corrupt_header_error>(
        m, "CorruptHeaderError", py_corrupt_record_error);
    register_exception<Corrupt_footer_error>(
        m, "CorruptFooterError", py_corrupt_record_error);
    register_exception<Record_too_large_error>(
        m, "RecordTooLargeError", py_record_error);

    PyObject *py_data_reader_error = register_exception<Data_reader_error>(
        m, "DataReaderError", py_mlio_error);
    register_exception<Schema_error>(
        m, "SchemaError", py_data_reader_error);
    register_exception<Invalid_instance_error>(
        m, "InvalidInstanceError", py_data_reader_error);

    register_exception<Not_supported_error>(
        m, "NotSupportedError", py_mlio_error);

    // clang-format on

    // NOLINTNEXTLINE
    py::register_exception_translator([](std::exception_ptr ptr) {
        if (!ptr) {
            return;
        }

        try {
            std::rethrow_exception(ptr);
        }
        catch (const Not_supported_error &e) {
            set_error(e, "NotSupportedError");
        }
        catch (const Invalid_instance_error &e) {
            set_error(e, "InvalidInstanceError");
        }
        catch (const Schema_error &e) {
            set_error(e, "SchemaError");
        }
        catch (const Data_reader_error &e) {
            set_error(e, "DataReaderError");
        }
        catch (const Record_too_large_error &e) {
            set_error(e, "RecordTooLargeError");
        }
        catch (const Corrupt_footer_error &e) {
            set_error(e, "CorruptFooterError");
        }
        catch (const Corrupt_header_error &e) {
            set_error(e, "CorruptHeaderError");
        }
        catch (const Corrupt_record_error &e) {
            set_error(e, "CorruptRecordError");
        }
        catch (const Record_error &e) {
            set_error(e, "RecordError");
        }
        catch (const Inflate_error &e) {
            set_error(e, "InflateError");
        }
        catch (const Stream_error &e) {
            set_error(e, "StreamError");
        }
        catch (const Mlio_error &e) {
            set_error(e, "MLIOError");
        }
        catch (const std::system_error &e) {
            set_system_error(e);
        }
    });
}

}  // namespace pymlio
