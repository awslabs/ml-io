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
inline namespace abi_v1 {
namespace detail {

// Reads data instances from a dataset.
class Instance_reader {
public:
    Instance_reader() noexcept = default;

    Instance_reader(const Instance_reader &) = delete;

    Instance_reader &operator=(const Instance_reader &) = delete;

    Instance_reader(Instance_reader &&) = delete;

    Instance_reader &operator=(Instance_reader &&) = delete;

    virtual ~Instance_reader();

    virtual std::optional<Instance> read_instance() = 0;

    virtual std::optional<Instance> peek_instance() = 0;

    virtual void reset() noexcept = 0;
};

using Record_reader_factory = std::function<Intrusive_ptr<Record_reader>(const Data_store &store)>;

std::unique_ptr<Instance_reader>
make_instance_reader(const Data_reader_params &params, Record_reader_factory &&factory);

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
