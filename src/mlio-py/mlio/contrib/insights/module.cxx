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

#include <cstddef>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <mlio.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "column_analyzer.h"
#include "column_statistics.h"

PYBIND11_DECLARE_HOLDER_TYPE(T, mlio::intrusive_ptr<T>, true);

namespace py = pybind11;

using namespace pybind11::literals;

namespace pymlio {
namespace {

data_analysis
analyze_dataset(mlio::data_reader &reader,
                std::unordered_set<std::string> const &null_like_values,
                std::unordered_set<size_t> const &capture_columns,
                std::size_t max_capture_count)
{
    mlio::intrusive_ptr<mlio::example> exm = reader.peek_example();
    if (exm == nullptr) {
        throw pybind11::stop_iteration(
            "Reached end of file without reading an example.");
    }

    // Set up the column_analsis objects to store the collected
    // information.
    std::vector<column_analysis> column_stats;
    column_stats.reserve(exm->features().size());

    for (mlio::feature_desc const &desc : exm->get_schema().descriptors()) {
        if (desc.sparse() || desc.dtype() != mlio::data_type::string) {
            throw std::runtime_error(
                "Data insights only works with dense string tensors.");
        }

        column_stats.emplace_back(desc.name());
    }

    std::vector<std::string> null_like_list{null_like_values.begin(),
                                            null_like_values.end()};

    column_analyzer analyzer{
        column_stats, null_like_list, capture_columns, max_capture_count};

    // Iterate over the entire dataset.
    while ((exm = reader.read_example())) {
        analyzer.analyze(*exm);
    }

    auto max_len = std::numeric_limits<std::size_t>::max();

    // Cleans up any temporary or intermediate placeholder values.
    for (auto &stats : column_stats) {
        if (stats.str_min_length == max_len) {
            stats.str_min_length = 0;
        }
        if (stats.str_min_length_not_empty == max_len) {
            stats.str_min_length_not_empty = 0;
        }
    }

    return data_analysis{std::move(column_stats)};
}

}  // namespace

PYBIND11_MODULE(insights, m)
{
    std::vector<std::pair<const char *, double column_analysis::*>>
        double_stat_names = {
            {"numeric_finite_mean", &column_analysis::numeric_finite_mean},
            {"numeric_finite_min", &column_analysis::numeric_finite_min},
            {"numeric_finite_max", &column_analysis::numeric_finite_max},
        };

    std::vector<std::pair<const char *, size_t column_analysis::*>>
        long_stat_names = {
            {"rows_seen", &column_analysis::rows_seen},
            {"numeric_count", &column_analysis::numeric_count},
            {"numeric_finite_count", &column_analysis::numeric_count},
            {"numeric_nan_count", &column_analysis::numeric_nan_count},
            {"string_empty_count", &column_analysis::str_empty_count},
            {"string_min_length", &column_analysis::str_min_length},
            {"string_min_length_not_empty",
             &column_analysis::str_min_length_not_empty},
            {"string_max_length", &column_analysis::str_max_length},
            {"string_only_whitespace_count",
             &column_analysis::str_only_whitespace_count},
            {"string_null_like_count", &column_analysis::str_null_like_count},
        };

    std::vector<std::pair<const char *, std::string column_analysis::*>>
        str_stat_names = {
            {"example_value", &column_analysis::example_value},
        };

    auto ca_class = py::class_<column_analysis>(m, "ColumnAnalysis");

    ca_class.def_readwrite("column_name", &column_analysis::column_name);
    ca_class.def_readwrite("string_captured_unique_values",
                           &column_analysis::str_captured_unique_values);
    ca_class.def_readwrite(
        "string_captured_unique_values_overflowed",
        &column_analysis::str_captured_unique_values_overflowed);

    for (auto &[name, method] : long_stat_names) {
        ca_class.def_readwrite(name, method);
    }
    for (auto &[name, method] : double_stat_names) {
        ca_class.def_readwrite(name, method);
    }
    for (auto &[name, method] : str_stat_names) {
        ca_class.def_readwrite(name, method);
    }

    ca_class.def("estimate_string_cardinality",
                 &column_analysis::estimate_string_cardinality);

    ca_class.def("to_dict", [=](column_analysis const &self) {
        py::dict result{};

        for (auto &[name, method] : long_stat_names) {
            std::size_t value = self.*method;
            result[name] = std::to_string(value);
        }
        for (auto &[name, method] : double_stat_names) {
            double value = self.*method;
            result[name] = std::to_string(value);
        }
        for (auto &[name, method] : str_stat_names) {
            result[name] = self.*method;
        }

        result["string_cardinality"] = self.estimate_string_cardinality();
        result["string_captured_unique_values"] =
            self.str_captured_unique_values;
        result["string_captured_unique_values_overflowed"] =
            self.str_captured_unique_values_overflowed;

        return result;
    });

    ca_class.def("__repr__", [](column_analysis const &self) {
        return "ColumnAnalysis(" + self.column_name + ")";
    });

    py::class_<data_analysis>(m, "DataAnalysis")
        .def_readwrite("columns", &data_analysis::columns);

    m.def("analyze_dataset",
          &analyze_dataset,
          "reader"_a,
          "null_like_values"_a,
          "capture_columns"_a,
          "max_capture_count"_a = 1000,
          "Analyzes a dataset");
}

}  // namespace pymlio
