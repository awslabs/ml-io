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

#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include <hyperloglog.h>

namespace pymlio {

class Column_analysis {
    friend class Column_analyzer;

private:
    static constexpr uint8_t cardinality_hill_size = 16;

public:
    explicit Column_analysis(std::string name)
        : column_name{std::move(name)}, str_cardinality_estimator_{cardinality_hill_size}
    {}

public:
    std::size_t estimate_string_cardinality() const
    {
        return static_cast<std::size_t>(std::round(str_cardinality_estimator_.estimate()));
    }

public:
    double estimate_median_approx() const
    {
        if (numeric_column_sample.empty()) {
            return std::nan("");
        }
        size_t n = numeric_column_sample.size() / 2;
        std::nth_element(numeric_column_sample.begin(), numeric_column_sample.begin() + n, 
            numeric_column_sample.end());
        return numeric_column_sample[n];
    }

public:
    std::string column_name;

    std::size_t rows_seen{};
    std::size_t numeric_count{};
    std::size_t numeric_nan_count{};
    std::size_t numeric_finite_count{};
    std::size_t numeric_int_count{};
    double numeric_finite_mean{};
    double numeric_finite_min = std::numeric_limits<double>::quiet_NaN();
    double numeric_finite_max = std::numeric_limits<double>::quiet_NaN();

    std::size_t str_min_length = std::numeric_limits<size_t>::max();
    std::size_t str_min_length_not_empty = std::numeric_limits<size_t>::max();
    std::size_t str_max_length{};
    std::size_t str_empty_count{};
    std::size_t str_only_whitespace_count{};
    std::size_t str_null_like_count{};
    std::unordered_map<std::string, int> str_captured_unique_values{};
    bool str_captured_unique_values_overflowed{};

    std::string example_value{};

private:
    hll::HyperLogLog str_cardinality_estimator_;
    mutable std::vector<double> numeric_column_sample{};
};

struct data_analysis {
    std::vector<Column_analysis> columns{};
};

}  // namespace pymlio
