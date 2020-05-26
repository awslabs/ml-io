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

#include <arrow/buffer.h>
#include <mlio/memory/memory_slice.h>

namespace pymlio {

class Arrow_buffer final : public arrow::Buffer {
public:
    explicit Arrow_buffer(mlio::Memory_slice s) noexcept;

    Arrow_buffer(const Arrow_buffer &) = delete;

    Arrow_buffer(Arrow_buffer &&) = delete;

    ~Arrow_buffer() final;

public:
    Arrow_buffer &operator=(const Arrow_buffer &) = delete;

    Arrow_buffer &operator=(Arrow_buffer &&) = delete;

private:
    mlio::Memory_slice slice_;
};

}  // namespace pymlio
