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

#include <optional>

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ref_counter.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup records Records
/// @{

class MLIO_API Record_reader : public Intrusive_ref_counter<Record_reader> {
public:
    Record_reader() noexcept = default;

    Record_reader(const Record_reader &) = delete;

    Record_reader &operator=(const Record_reader &) = delete;

    Record_reader(Record_reader &&) = delete;

    Record_reader &operator=(Record_reader &&) = delete;

    virtual ~Record_reader();

    virtual std::optional<Record> read_record() = 0;

    virtual std::optional<Record> peek_record() = 0;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
