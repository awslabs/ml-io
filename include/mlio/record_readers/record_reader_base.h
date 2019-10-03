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
#include "mlio/optional.h"
#include "mlio/record_readers/record.h"
#include "mlio/record_readers/record_reader.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup records Records
/// @{

class MLIO_API record_reader_base : public record_reader {
public:
    stdx::optional<record>
    read_record() final;

    stdx::optional<record> const &
    peek_record() final;

private:
    stdx::optional<record>
    read_record_internal();

    virtual stdx::optional<record>
    read_record_core() = 0;

private:
    stdx::optional<record> peeked_record_{};
};

/// @}

}  // namespace v1
}  // namespace mlio
