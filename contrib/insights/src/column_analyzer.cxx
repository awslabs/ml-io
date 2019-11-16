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

#include "mlio.h"
#include "tbb/tbb.h"

#include "column_analyzer.h"
#include "utils.h"

using namespace tbb;

namespace insights {
namespace detail {

column_analyzer::column_analyzer(
    mlio::intrusive_ptr<mlio::example> *current_example,
    std::vector<column_analysis> &columns,
    std::unordered_set<std::string> const &null_like_values,
    std::unordered_set<std::size_t> const &capture_columns,
    std::size_t max_capture_count)
    : current_example_(current_example)
    , columns_(&columns)
    , capture_columns_(&capture_columns)
    , max_capture_count_(max_capture_count)
{
    null_like_values_.insert(null_like_values_.end(),
                             null_like_values.begin(),
                             null_like_values.end());
}

void
column_analyzer::operator()(const blocked_range<std::size_t> &rngs) const
{
    for (auto feature_idx = rngs.begin(); feature_idx != rngs.end();
         ++feature_idx) {
        auto tsr = (*current_example_)->features()[feature_idx];
        column_analysis &feature_statistics = (*columns_)[feature_idx];

        auto dt = static_cast<mlio::dense_tensor *>(tsr.get());
        auto cells = dt->data().as<std::string>();

        auto should_capture =
            capture_columns_->find(feature_idx) != capture_columns_->end();

        // Used to update the mean.
        // We do this here to aggregate several entries together and avoid
        // potential numerical problems updating the mean a single entry at a
        // time.
        double numeric_column_sum = 0.0;
        double numeric_column_count = 0.0;

        for (std::string_view as_string : cells) {
            // Capture the first example.
            if (feature_statistics.rows_seen == 0) {
                feature_statistics.example_value = as_string;
            }

            feature_statistics.rows_seen += 1;

            // String analyzers.
            feature_statistics.string_min_length = std::min(
                feature_statistics.string_min_length, as_string.size());

            if (as_string.size() == 0) {
                feature_statistics.string_empty_count += 1;
                // All other analysis is irrelevant if we have an empty string.
                continue;
            }
            else {
                feature_statistics.string_cardinality_estimator.add(as_string);

                feature_statistics.string_min_length_not_empty =
                    std::min(feature_statistics.string_min_length_not_empty,
                             as_string.size());

                feature_statistics.string_max_length = std::max(
                    feature_statistics.string_max_length, as_string.size());
            }

            if (mlio::is_only_whitespace(as_string)) {
                feature_statistics.string_only_whitespace_count += 1;
                // All other analysis is irrelevant if we only have whitespace.
                continue;
            }

            if (details::match_nan_values(as_string, null_like_values_)) {
                feature_statistics.string_null_like_count += 1;
            }

            // Numeric analyzers
            double as_float;
            if (mlio::try_parse_float(mlio::v1::float_parse_params{as_string},
                                      as_float) != mlio::parse_result::ok) {
                feature_statistics.numeric_nan_count += 1;
            }
            else {
                feature_statistics.numeric_count += 1;
                if (!std::isnan(as_float) && !std::isinf(as_float)) {
                    feature_statistics.numeric_finite_count += 1;
                    numeric_column_sum += as_float;
                    numeric_column_count += 1;
                    if (isnan(feature_statistics.numeric_finite_min) ||
                        as_float < feature_statistics.numeric_finite_min) {
                        feature_statistics.numeric_finite_min = as_float;
                    }
                    if (isnan(feature_statistics.numeric_finite_max) ||
                        as_float > feature_statistics.numeric_finite_max) {
                        feature_statistics.numeric_finite_max = as_float;
                    }
                }
            }

            // Capture the values if specified.
            if (should_capture &&
                !feature_statistics.string_captured_unique_values_overflowed) {
                if (feature_statistics.string_captured_unique_values.size() <
                    max_capture_count_) {
                    feature_statistics.string_captured_unique_values.emplace(
                        std::string(as_string));
                }
                else if (feature_statistics.string_captured_unique_values.find(
                             std::string(as_string)) ==
                         feature_statistics.string_captured_unique_values
                             .end()) {
                    // If the value isn't present but we're not adding it
                    // because we're at a limit then we should flag that we
                    // have overflowed.
                    feature_statistics
                        .string_captured_unique_values_overflowed = true;
                }
            }
        }

        // Update the mean of numeric values based on the entire range of
        // values.
        feature_statistics.numeric_finite_mean +=
            ((numeric_column_sum / numeric_column_count) -
             feature_statistics.numeric_finite_mean) *
            numeric_column_count / feature_statistics.numeric_finite_count;
    }
};

}  // namespace detail
}  // namespace insights
