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

#include "column_statistics.h"
#include "mlio.h"

namespace insights {
namespace detail {

class MLIO_API column_analyzer {
public:
    column_analyzer(mlio::intrusive_ptr<mlio::example> *current_example,
                    std::vector<column_analysis> &columns,
                    std::unordered_set<std::string> const &null_like_values,
                    std::unordered_set<std::size_t> const &capture_columns,
                    std::size_t max_capture_count);

    void
    operator()(const tbb::blocked_range<std::size_t> &r) const;

private:
    mlio::intrusive_ptr<mlio::example> *current_example_;
    std::vector<column_analysis> *columns_;
    std::unordered_set<size_t> const *capture_columns_;
    std::vector<std::string> null_like_values_;
    std::size_t max_capture_count_;
};

}  // namespace detail
}  // namespace insights
