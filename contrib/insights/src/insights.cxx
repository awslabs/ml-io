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

#include <cmath>
#include <exception>
#include <functional>
#include <limits>
#include <map>
#include <string>
#include <utility>

#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include "tbb/tbb.h"

#include "column_analyzer.h"
#include "column_statistics.h"
#include "insights.h"
#include "module.h"

namespace py = pybind11;

using namespace tbb;
using namespace pybind11::literals;

namespace insights {
namespace detail {

pybind11::object
analyze_dataset(mlio::data_reader *reader_,
                std::unordered_set<std::string> const *null_like_values,
                std::unordered_set<size_t> const *capture_columns,
                std::size_t max_capture_count)
{
    // Stores the data that our parallel function will operate on.
    mlio::intrusive_ptr<mlio::example> exm = nullptr;
    std::vector<column_analysis> columns;

    // Analyzer that is used in the parallel function.
    auto analyzer = column_analyzer(
        &exm, columns, *null_like_values, *capture_columns, max_capture_count);

    // First set up the column_analsis objects to store the
    // collected information.
    exm = reader_->peek_example();
    if (exm == nullptr) {
        throw pybind11::stop_iteration(
            "Reached end of file without reading an example");
    }
    if (columns.empty()) {
        columns.reserve(exm->features().size());
        for (mlio::feature_desc const &desc :
             exm->get_schema().descriptors()) {
            column_analysis statistics = column_analysis(desc.name());
            columns.emplace_back(std::move(statistics));
        }
        // Check each tensor only contains strings.
        for (mlio::intrusive_ptr<mlio::tensor> t : exm->features()) {
            if (t->dtype() != mlio::data_type::string) {
                throw std::runtime_error(
                    "Data insights only works with string tensors.");
            }
        }
    }

    // Iterate over the entire dataset.
    while ((exm = reader_->read_example()) != nullptr) {
        exm = reader_->read_example();
        parallel_for(blocked_range<std::size_t>(0, columns.size()), analyzer);
    }

    // Finalize any collected metrics.
    for (auto column : columns) {
        column.finalize();
    }
    return pybind11::cast(detail::data_analysis{std::move(columns)});
}

}  // namespace detail

void
register_insights(py::module &m)
{
    std::vector<std::pair<const char *, double detail::column_analysis::*>>
        double_statistic_names = {
            std::make_pair(
                "numeric_finite_mean",
                &insights::detail::column_analysis::numeric_finite_mean),
            std::make_pair(
                "numeric_finite_min",
                &insights::detail::column_analysis::numeric_finite_min),
            std::make_pair(
                "numeric_finite_max",
                &insights::detail::column_analysis::numeric_finite_max),
        };

    std::vector<std::pair<const char *, size_t detail::column_analysis::*>>
        long_statistic_names = {
            std::make_pair("rows_seen",
                           &insights::detail::column_analysis::rows_seen),
            std::make_pair("numeric_count",
                           &insights::detail::column_analysis::numeric_count),
            std::make_pair("numeric_finite_count",
                           &insights::detail::column_analysis::numeric_count),
            std::make_pair(
                "numeric_nan_count",
                &insights::detail::column_analysis::numeric_nan_count),
            std::make_pair(
                "string_empty_count",
                &insights::detail::column_analysis::string_empty_count),
            std::make_pair(
                "string_min_length",
                &insights::detail::column_analysis::string_min_length),
            std::make_pair("string_min_length_not_empty",
                           &insights::detail::column_analysis::
                               string_min_length_not_empty),
            std::make_pair(
                "string_max_length",
                &insights::detail::column_analysis::string_max_length),
            std::make_pair("string_only_whitespace_count",
                           &insights::detail::column_analysis::
                               string_only_whitespace_count),
            std::make_pair(
                "string_null_like_count",
                &insights::detail::column_analysis::string_null_like_count),
        };

    std::vector<
        std::pair<const char *, std::string detail::column_analysis::*>>
        string_statistic_names = {
            std::make_pair("example_value",
                           &insights::detail::column_analysis::example_value),
        };

    auto ca_class =
        py::class_<insights::detail::column_analysis>(m, "ColumnAnalysis");
    ca_class.def_readwrite("column_name",
                           &insights::detail::column_analysis::column_name);
    ca_class.def("string_cardinality",
                 [](const insights::detail::column_analysis &ca) {
                     return ca.estimate_string_cardinality();
                 });
    ca_class.def_readwrite(
        "string_captured_unique_values",
        &insights::detail::column_analysis::string_captured_unique_values);
    ca_class.def_readwrite("string_captured_unique_values_overflowed",
                           &insights::detail::column_analysis::
                               string_captured_unique_values_overflowed);

    for (auto name_method_pair : long_statistic_names) {
        auto name = name_method_pair.first;
        auto method = name_method_pair.second;
        ca_class.def_readwrite(name, method);
    }

    for (auto name_method_pair : double_statistic_names) {
        auto name = name_method_pair.first;
        auto method = name_method_pair.second;
        ca_class.def_readwrite(name, method);
    }

    for (auto name_method_pair : string_statistic_names) {
        auto name = name_method_pair.first;
        auto method = name_method_pair.second;
        ca_class.def_readwrite(name, method);
    }

    ca_class.def("to_dict", [=](const insights::detail::column_analysis &ca) {
        py::dict result;

        for (auto &[name, method] : long_statistic_names) {
            std::size_t value = ca.*method;
            result[name] = std::to_string(value);
        }

        for (auto &[name, method] : double_statistic_names) {
            double value = (ca.*method);
            result[name] = std::to_string(value);
        }

        for (auto &[name, method] : string_statistic_names) {
            std::string value = (ca.*method);
            result[name] = value;
        }

        result["string_cardinality"] = ca.estimate_string_cardinality();
        result["string_captured_unique_values"] =
            ca.string_captured_unique_values;
        result["string_captured_unique_values_overflowed"] =
            ca.string_captured_unique_values_overflowed;

        return result;
    });

    ca_class.def("__repr__", [](const insights::detail::column_analysis &ca) {
        return "ColumnAnalysis(" + ca.column_name + ")";
    });

    py::class_<insights::detail::data_analysis>(m, "DataAnalysis")
        .def_readwrite("columns", &insights::detail::data_analysis::columns);

    m.def("analyze_dataset",
          &insights::detail::analyze_dataset,
          "Analyzes a dataset");
}

}  // namespace insights
