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
#include <functional>
#include <vector>

#include "mlio/data_stores/data_store.h"
#include "mlio/fwd.h"
#include "mlio/instance_reader_base.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/optional.h"
#include "mlio/record_readers/record_reader.h"

namespace mlio {
inline namespace v1 {
namespace detail {

using record_reader_factory =
    std::function<intrusive_ptr<record_reader>(data_store const &ds)>;

class default_instance_reader final : public instance_reader_base {
public:
    explicit
    default_instance_reader(data_reader_params const &prm,
                            record_reader_factory &&fct) noexcept;

private:
    stdx::optional<instance>
    read_instance_core() final;

    bool
    should_stop_reading() const noexcept;

    bool
    skip_instances();

    stdx::optional<instance>
    read_instance_internal();

    [[noreturn]]
    void
    handle_nested_errors();

    stdx::optional<memory_slice>
    read_record_payload();

    stdx::optional<record>
    read_record();

    bool
    init_next_record_reader();

public:
    void
    reset() noexcept final;

private:
    data_reader_params const *params_;
    record_reader_factory record_reader_factory_;
    std::vector<intrusive_ptr<data_store>>::const_iterator store_iter_;
    data_store *store_{};
    intrusive_ptr<record_reader> record_reader_{};
    std::size_t record_idx_{};
    std::size_t instance_idx_{};
    std::size_t num_instances_skipped_{};
    std::size_t num_instances_read_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
