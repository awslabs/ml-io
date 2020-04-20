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

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

class py_log_message_handler {
public:
    explicit py_log_message_handler(py::function hdl) noexcept
        : hdl_{std::move(hdl)}
    {}

    py_log_message_handler(py_log_message_handler const &) = default;

    py_log_message_handler(py_log_message_handler &&) = delete;

    ~py_log_message_handler()
    {
        py::gil_scoped_acquire acq_gil;

        hdl_ = {};
    }

public:
    py_log_message_handler &
    operator=(py_log_message_handler const &) = default;

    py_log_message_handler &
    operator=(py_log_message_handler &&) = delete;

public:
    void
    operator()(log_level lvl, std::string_view msg) const
    {
        py::gil_scoped_acquire acq_gil;

        hdl_(lvl, msg);
    }

private:
    py::function hdl_;
};

log_message_handler
py_set_log_message_handler(py::function hdl)
{
    py_log_message_handler py_hdl{std::move(hdl)};

    return set_log_message_handler(
        [py_hdl](log_level lvl, std::string_view msg) {
            py_hdl(lvl, msg);
        });
}

}  // namespace

void
register_logging(py::module &m)
{
    py::enum_<log_level>(m, "LogLevel")
        .value("OFF", log_level::off)
        .value("WARNING", log_level::warning)
        .value("INFO", log_level::info)
        .value("DEBUG", log_level::debug);

    m.def("set_log_message_handler",
          &py_set_log_message_handler,
          "hdl"_a,
          "Sets the logging handler.");

    m.def("clear_log_message_handler", []() {
        set_log_message_handler(nullptr);
    });

    m.def("set_log_level", &set_log_level, "lvl"_a, "Sets the log level.");
}

}  // namespace pymlio
