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

#include "mlio/config.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ref_counter.h"
#include "mlio/optional.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup records Records
/// @{

class MLIO_API record_reader : public intrusive_ref_counter<record_reader> {
public:
    record_reader() noexcept = default;

    record_reader(record_reader const &) = delete;

    record_reader(record_reader &&) = delete;

    virtual
   ~record_reader();

public:
    record_reader &
    operator=(record_reader const &) = delete;

    record_reader &
    operator=(record_reader &&) = delete;

public:
    virtual stdx::optional<record>
    read_record() = 0;

    virtual stdx::optional<record> const &
    peek_record() = 0;
};

/// @}

}  // namespace v1
}  // namespace mlio
