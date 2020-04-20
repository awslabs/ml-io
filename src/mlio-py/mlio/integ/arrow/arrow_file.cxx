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

#include "arrow_file.h"

#include <stdexcept>
#include <utility>

#include <arrow/result.h>
#include <arrow/status.h>

#include "arrow_buffer.h"
#include "arrow_util.h"

using namespace mlio;

namespace pymlio {

arrow_file::arrow_file(intrusive_ptr<input_stream> strm) : stream_{std::move(strm)}
{
    if (!stream_->seekable()) {
        throw std::invalid_argument{"The input stream is not seekable."};
    }
}

arrow_file::~arrow_file() = default;

arrow::Result<std::int64_t> arrow_file::Read(std::int64_t nbytes, void *out) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::int64_t>([=]() {
        auto size = static_cast<std::size_t>(nbytes);

        auto bits = static_cast<std::byte *>(out);

        mutable_memory_span dest{bits, size};

        return static_cast<std::int64_t>(stream_->read(dest));
    });
}

arrow::Result<std::shared_ptr<arrow::Buffer>> arrow_file::Read(std::int64_t nbytes) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::shared_ptr<arrow::Buffer>>([=]() {
        auto size = static_cast<std::size_t>(nbytes);

        return std::make_shared<arrow_buffer>(stream_->read(size));
    });
}

arrow::Status arrow_file::Seek(std::int64_t position) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary([=]() {
        stream_->seek(static_cast<std::size_t>(position));
    });
}

arrow::Status arrow_file::Close() noexcept
{
    stream_->close();

    return arrow::Status::OK();
}

arrow::Result<std::int64_t> arrow_file::Tell() const noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::int64_t>([=]() {
        return static_cast<std::int64_t>(stream_->position());
    });
}

arrow::Result<std::int64_t> arrow_file::GetSize() noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::int64_t>([=]() {
        return static_cast<std::int64_t>(stream_->size());
    });
}

arrow::Status arrow_file::check_if_closed() const noexcept
{
    if (stream_->closed()) {
        return arrow::Status::Invalid("Invalid operation on closed file.");
    }
    return arrow::Status::OK();
}

bool arrow_file::supports_zero_copy() const noexcept
{
    return stream_->supports_zero_copy();
}

bool arrow_file::closed() const noexcept
{
    return stream_->closed();
}

}  // namespace pymlio
