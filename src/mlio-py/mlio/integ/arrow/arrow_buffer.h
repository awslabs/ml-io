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

#include <arrow/buffer.h>
#include <mlio.h>

namespace mliopy {

class arrow_buffer final : public arrow::Buffer {
public:
    explicit
    arrow_buffer(mlio::memory_slice s) noexcept;

    arrow_buffer(arrow_buffer const &) = delete;

    arrow_buffer(arrow_buffer &&) = delete;

   ~arrow_buffer() final;

public:
    arrow_buffer &
    operator=(arrow_buffer const &) = delete;

    arrow_buffer &
    operator=(arrow_buffer &&) = delete;

private:
    mlio::memory_slice slice_;
};

}  // namespace mliopy
