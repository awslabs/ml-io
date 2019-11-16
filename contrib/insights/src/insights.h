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

#include <cstddef>
#include <string>
#include <unordered_set>

#include "module.h"

namespace insights {
namespace detail {

pybind11::object
analyze_dataset(mlio::data_reader *reader_,
                std::unordered_set<std::string> const *null_like_values,
                std::unordered_set<std::size_t> const *capture_columns,
                std::size_t max_capture_count);

}  // namespace detail
}  // namespace insights
