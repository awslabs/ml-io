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
#include <optional>

#include "mlio/instance.h"
#include "mlio/instance_reader.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class instance_reader_base : public instance_reader {
public:
    std::optional<instance>
    read_instance() final;

    std::optional<instance>
    peek_instance() final;

    void
    reset() noexcept final;

private:
    virtual std::optional<instance>
    read_instance_core() = 0;

    virtual void
    reset_core() noexcept = 0;

public:
    std::size_t
    num_bytes_read() const noexcept final
    {
        return num_bytes_read_;
    }

private:
    std::optional<instance> peeked_instance_{};
    std::size_t num_bytes_read_{};
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
