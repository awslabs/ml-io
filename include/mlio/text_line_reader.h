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
#include <vector>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/parallel_data_reader.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a @ref data_reader for reading simple text-based datasets.
class MLIO_API text_line_reader final : public parallel_data_reader {
public:
    explicit text_line_reader(data_reader_params prm);

    text_line_reader(text_line_reader const &) = delete;

    text_line_reader(text_line_reader &&) = delete;

    ~text_line_reader() final;

public:
    text_line_reader &
    operator=(text_line_reader const &) = delete;

    text_line_reader &
    operator=(text_line_reader &&) = delete;

private:
    intrusive_ptr<record_reader>
    make_record_reader(data_store const &ds) final;

    intrusive_ptr<schema const>
    infer_schema(std::optional<instance> const &ins) final;

    intrusive_ptr<example>
    decode(instance_batch const &batch) const final;

    static intrusive_ptr<dense_tensor>
    make_tensor(std::size_t batch_size);
};

/// @}

}  // namespace v1
}  // namespace mlio
