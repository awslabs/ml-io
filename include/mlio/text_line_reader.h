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
#include <vector>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/parallel_data_reader.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a @ref Data_reader for reading simple text-based datasets.
class MLIO_API Text_line_reader final : public Parallel_data_reader {
public:
    explicit Text_line_reader(Data_reader_params params);

    Text_line_reader(const Text_line_reader &) = delete;

    Text_line_reader &operator=(const Text_line_reader &) = delete;

    Text_line_reader(Text_line_reader &&) = delete;

    Text_line_reader &operator=(Text_line_reader &&) = delete;

    ~Text_line_reader() final;

private:
    Intrusive_ptr<Record_reader> make_record_reader(const Data_store &store) final;

    Intrusive_ptr<const Schema> infer_schema(const std::optional<Instance> &instance) final;

    Intrusive_ptr<Example> decode(const Instance_batch &batch) const final;

    static Intrusive_ptr<Dense_tensor> make_tensor(std::size_t batch_size);
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
