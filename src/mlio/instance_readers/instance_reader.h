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

#include <functional>
#include <memory>
#include <optional>

#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace v1 {
namespace detail {

// Reads data instances from a dataset.
class instance_reader {
public:
    instance_reader() noexcept = default;

    instance_reader(const instance_reader &) = delete;

    instance_reader(instance_reader &&) = delete;

    virtual ~instance_reader();

public:
    instance_reader &operator=(const instance_reader &) = delete;

    instance_reader &operator=(instance_reader &&) = delete;

public:
    virtual std::optional<instance> read_instance() = 0;

    virtual std::optional<instance> peek_instance() = 0;

    virtual void reset() noexcept = 0;
};

using record_reader_factory = std::function<intrusive_ptr<record_reader>(const data_store &ds)>;

std::unique_ptr<instance_reader>
make_instance_reader(const data_reader_params &prm, record_reader_factory &&fct);

}  // namespace detail
}  // namespace v1
}  // namespace mlio
