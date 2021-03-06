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
#include <utility>

#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/record_readers/stream_record_reader.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

class Recordio_record_reader final : public Stream_record_reader {
public:
    explicit Recordio_record_reader(Intrusive_ptr<Input_stream> stream)
        : Stream_record_reader{std::move(stream)}
    {}

private:
    std::optional<Record> decode_record(Memory_slice &chunk, bool ignore_leftover) final;
};

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
