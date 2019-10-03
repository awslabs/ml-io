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

#include <utility>

#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/optional.h"
#include "mlio/record_readers/text_record_reader.h"
#include "mlio/streams/input_stream.h"

namespace mlio {
inline namespace v1 {
namespace detail {

class text_line_record_reader final : public text_record_reader {
public:
    explicit
    text_line_record_reader(intrusive_ptr<input_stream> strm, bool skip_blank)
        : text_record_reader{std::move(strm)}, skip_blank_{skip_blank}
    {}

private:
    stdx::optional<record>
    decode_text_record(memory_slice &chunk, bool ignore_leftover) final;

private:
    bool skip_blank_;
};

}  // namespace detail
}  // namespace v1
}  // namespace mlio
