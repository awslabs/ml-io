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

#include "column_analyzer.h"

#include <cmath>
#include <string_view>

#include "utils.h"

namespace pymlio {

column_analyzer::column_analyzer(std::vector<column_analysis> &columns,
                                 std::vector<std::string> const &null_like_values,
                                 std::unordered_set<std::size_t> const &capture_columns,
                                 std::size_t max_capture_count) noexcept
    : columns_{&columns}
    , null_like_values_{&null_like_values}
    , capture_columns_{&capture_columns}
    , max_capture_count_{max_capture_count}
{}

void column_analyzer::analyze(const mlio::example &exm) const
{
    std::size_t feature_idx = 0;

    for (auto pos = exm.features().begin(); pos < exm.features().end(); ++pos, feature_idx++) {
        const auto &tsr = exm.features()[feature_idx];
        const auto &dense_tsr = static_cast<const mlio::dense_tensor &>(*tsr);
        auto cells = dense_tsr.data().as<std::string>();

        column_analysis &stats = (*columns_)[feature_idx];

        // Used to update the mean.
        // We do this here to aggregate several entries together and avoid
        // potential numerical problems updating the mean a single entry at a
        // time.
        double numeric_column_sum = 0.0;
        std::size_t numeric_column_count = 0.0;

        for (const std::string &cell : cells) {
            // Capture the first example.
            if (stats.rows_seen == 0) {
                stats.example_value = cell;
            }

            stats.rows_seen++;

            // String analyzers.
            if (cell.empty()) {
                stats.str_empty_count++;

                // All other analysis is irrelevant if we have an empty string.
                continue;
            }

            stats.str_min_length = std::min(stats.str_min_length, cell.size());
            stats.str_max_length = std::max(stats.str_max_length, cell.size());

            stats.str_min_length_not_empty = std::min(stats.str_min_length_not_empty, cell.size());

            stats.str_cardinality_estimator_.add(cell);

            if (mlio::is_whitespace_only(cell)) {
                stats.str_only_whitespace_count++;

                // All other analysis is irrelevant if we only have whitespace.
                continue;
            }

            if (match_nan_values(cell, *null_like_values_)) {
                stats.str_null_like_count++;
            }

            // Numeric analyzers
            double as_float{};
            if (mlio::try_parse_float(cell, as_float) != mlio::parse_result::ok) {
                stats.numeric_nan_count++;
            }
            else {
                stats.numeric_count++;
                if (!std::isnan(as_float) && !std::isinf(as_float)) {
                    stats.numeric_finite_count++;

                    numeric_column_sum += as_float;
                    numeric_column_count++;

                    if (std::isnan(stats.numeric_finite_min) ||
                        as_float < stats.numeric_finite_min) {
                        stats.numeric_finite_min = as_float;
                    }
                    if (std::isnan(stats.numeric_finite_max) ||
                        as_float > stats.numeric_finite_max) {
                        stats.numeric_finite_max = as_float;
                    }
                }
            }

            auto should_capture = capture_columns_->find(feature_idx) != capture_columns_->end();

            // Capture the values if specified.
            if (should_capture && !stats.str_captured_unique_values_overflowed) {
                if (stats.str_captured_unique_values.size() < max_capture_count_) {
                    stats.str_captured_unique_values.emplace(cell);
                }
                else if (stats.str_captured_unique_values.find(cell) ==
                         stats.str_captured_unique_values.end()) {
                    // If the value isn't present but we're not adding it
                    // because we're at a limit then we should flag that we
                    // have overflowed.
                    stats.str_captured_unique_values_overflowed = true;
                }
            }
        }

        // Update the mean of numeric values based on the entire range of
        // values.
        auto ncc = static_cast<double>(numeric_column_count);
        auto nfc = static_cast<double>(stats.numeric_finite_count);

        double numeric_column_mean = numeric_column_sum / ncc;

        stats.numeric_finite_mean += (numeric_column_mean - stats.numeric_finite_mean) * ncc / nfc;
    }
};

}  // namespace pymlio
