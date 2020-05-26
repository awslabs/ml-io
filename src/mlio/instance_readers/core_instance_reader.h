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

#include <cstddef>
#include <optional>
#include <vector>

#include "mlio/data_stores/data_store.h"
#include "mlio/fwd.h"
#include "mlio/instance_readers/instance_reader.h"
#include "mlio/instance_readers/instance_reader_base.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/record_readers/record_reader.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Core_instance_reader final : public Instance_reader_base {
public:
    explicit Core_instance_reader(const Data_reader_params &params,
                                  Record_reader_factory &&factory);

private:
    std::optional<Instance> read_instance_core() final;

    [[noreturn]] void handle_errors();

    std::optional<Memory_slice> read_record_payload();

    std::optional<Memory_slice> read_split_record_payload(std::optional<Record> record);

    [[noreturn]] void throw_corrupt_split_record_error();

    std::optional<Record> read_record();

    bool init_next_record_reader();

    void reset_core() noexcept final;

    const Data_reader_params *params_;
    Record_reader_factory record_reader_factory_;
    std::vector<Intrusive_ptr<Data_store>>::const_iterator store_iter_{};
    Data_store *store_{};
    Intrusive_ptr<Record_reader> record_reader_{};
    std::size_t instance_idx_{};
    std::size_t record_idx_{};
    bool has_corrupt_split_record_{};
};

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
