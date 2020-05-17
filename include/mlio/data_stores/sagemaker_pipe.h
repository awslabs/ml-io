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

#include <chrono>
#include <cstddef>
#include <optional>
#include <string>

#include "mlio/config.h"
#include "mlio/data_stores/compression.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/streams/sagemaker_pipe_input_stream.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Represents an Amazon SageMaker pipe channel as a @ref data_store.
class MLIO_API sagemaker_pipe final : public data_store {
public:
    explicit sagemaker_pipe(std::string pathname,
                            std::chrono::seconds timeout = sagemaker_pipe_default_timeout,
                            std::optional<std::size_t> fifo_id = {},
                            compression cmp = {});

public:
    intrusive_ptr<input_stream> open_read() const final;

    std::string repr() const final;

public:
    const std::string &id() const noexcept final
    {
        return pathname_;
    }

private:
    std::string pathname_;
    std::chrono::seconds timeout_;
    mutable std::optional<std::size_t> fifo_id_;
    compression compression_;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
