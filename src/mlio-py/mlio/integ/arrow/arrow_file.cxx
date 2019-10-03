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

#include "integ/arrow/arrow_file.h"

#include <stdexcept>
#include <utility>

#include <arrow/status.h>

#include "integ/arrow/arrow_buffer.h"
#include "integ/arrow/arrow_util.h"

namespace stdx = mlio::stdx;

namespace mliopy {

arrow_file::
arrow_file(mlio::intrusive_ptr<mlio::input_stream> strm)
    : stream_{std::move(strm)}
{
    if (!stream_->seekable()) {
        throw std::invalid_argument{"The input stream is not seekable."};
    }
}

arrow_file::~arrow_file() = default;

arrow::Status
arrow_file::
Read(std::int64_t nbytes, std::int64_t *bytes_read, void *out) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary([=]() {
        auto size = static_cast<std::size_t>(nbytes);

        auto bits = static_cast<stdx::byte *>(out);

        mlio::mutable_memory_span dest{bits, size};

        std::size_t num_bytes_read = stream_->read(dest);

        *bytes_read = static_cast<std::int64_t>(num_bytes_read);
    });
}

arrow::Status
arrow_file::
Read(std::int64_t nbytes, std::shared_ptr<arrow::Buffer> *out) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary([=]() {
        auto size = static_cast<std::size_t>(nbytes);

        *out = std::make_shared<arrow_buffer>(stream_->read(size));
    });
}

arrow::Status
arrow_file::
Seek(std::int64_t position) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary([=]() {
        stream_->seek(static_cast<std::size_t>(position));
    });
}

arrow::Status
arrow_file::
Close() noexcept
{
    stream_->close();

    return arrow::Status::OK();
}

arrow::Status
arrow_file::
Tell(std::int64_t *position) const noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary([=]() {
        *position = static_cast<std::int64_t>(stream_->position());
    });
}

arrow::Status
arrow_file::
GetSize(std::int64_t *size) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary([=]() {
        *size = static_cast<std::int64_t>(stream_->size());
    });
}

arrow::Status
arrow_file::
check_if_closed() const noexcept
{
    if (stream_->closed()) {
        return arrow::Status::Invalid("Invalid operation on closed file.");
    }
    return arrow::Status::OK();
}

bool
arrow_file::
supports_zero_copy() const noexcept
{
    return stream_->supports_zero_copy();
}

bool
arrow_file::
closed() const noexcept
{
    return stream_->closed();
}

}  // namespace mliopy
