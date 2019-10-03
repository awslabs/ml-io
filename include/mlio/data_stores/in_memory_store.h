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

#include <string>

#include "mlio/config.h"
#include "mlio/data_stores/compression.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/memory/memory_slice.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_stores Data Stores
/// @{

/// Represents a memory block as a @ref data_store.
class MLIO_API in_memory_store final : public data_store {
public:
    explicit
    in_memory_store(memory_slice chunk, compression cmp = {});

public:
    intrusive_ptr<input_stream>
    open_read() const final;

    std::string
    repr() const final;

public:
    std::string const &
    id() const noexcept final
    {
        return id_;
    }

private:
    std::string id_;
    memory_slice chunk_;
    compression compression_;
};

/// @}

}  // namespace v1
}  // namespace mlio
