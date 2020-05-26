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

#include <cstddef>
#include <string>

#include "mlio/config.h"
#include "mlio/detail/file_descriptor.h"
#include "mlio/span.h"
#include "mlio/streams/input_stream_base.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup streams Streams
/// @{

class MLIO_API File_input_stream final : public Input_stream_base {
public:
    explicit File_input_stream(std::string path);

    using Input_stream_base::read;

    std::size_t read(Mutable_memory_span destination) final;

    void seek(std::size_t position) final;

    void close() noexcept final;

    std::size_t size() const final;

    std::size_t position() const final;

    bool closed() const noexcept final
    {
        return !fd_.is_open();
    }

    bool seekable() const noexcept final
    {
        return true;
    }

private:
    MLIO_HIDDEN
    void check_if_closed() const;

    std::string path_;
    detail::File_descriptor fd_{};
    mutable std::size_t size_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
