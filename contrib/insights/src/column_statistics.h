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

#include "hll/hyperloglog.h"

namespace insights {
namespace detail {

class column_analysis {
public:
    column_analysis(std::string name) : column_name{name}
    {
        string_cardinality_estimator = hll::HyperLogLog(CARDINALITY_HLL_SIZE);
    }

    std::size_t
    estimate_string_cardinality() const
    {
        return static_cast<std::size_t>(
            std::round(string_cardinality_estimator.estimate()));
    }

    /// Cleans up any temporary or intermediate placeholder values.
    void
    finalize()
    {
        if (string_min_length == std::numeric_limits<std::size_t>::max()) {
            string_min_length = 0;
        }
        if (string_min_length_not_empty ==
            std::numeric_limits<std::size_t>::max()) {
            string_min_length_not_empty = 0;
        }
    }

    std::string column_name;

    std::size_t rows_seen{};
    std::size_t numeric_count{};
    std::size_t numeric_nan_count{};
    std::size_t numeric_finite_count{};
    double numeric_finite_mean{};
    double numeric_finite_min{std::numeric_limits<double>::quiet_NaN()};
    double numeric_finite_max{std::numeric_limits<double>::quiet_NaN()};

    std::size_t string_min_length{std::numeric_limits<size_t>::max()};
    std::size_t string_min_length_not_empty{
        std::numeric_limits<size_t>::max()};
    std::size_t string_max_length{};
    std::size_t string_empty_count{};
    std::size_t string_only_whitespace_count{};
    std::size_t string_null_like_count{};
    std::unordered_set<std::string> string_captured_unique_values{};
    bool string_captured_unique_values_overflowed{};
    hll::HyperLogLog string_cardinality_estimator;

    std::string example_value{};

private:
    static const uint8_t CARDINALITY_HLL_SIZE = 16;
};

struct data_analysis {
    std::vector<column_analysis> columns;
};

}  // namespace detail
}  // namespace insights
