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
#include <mlio/span.h>

#include "arrow_buffer.h"
#include "arrow_util.h"

using namespace mlio;

namespace pymlio {

Arrow_file::Arrow_file(Intrusive_ptr<Input_stream> stream) : stream_{std::move(stream)}
{
    if (!stream_->seekable()) {
        throw std::invalid_argument{"The input stream is not seekable."};
    }
}

Arrow_file::~Arrow_file() = default;

arrow::Result<std::int64_t> Arrow_file::Read(std::int64_t nbytes, void *out) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::int64_t>([=]() {
        auto size = static_cast<std::size_t>(nbytes);

        auto bits = static_cast<std::byte *>(out);

        Mutable_memory_span destination{bits, size};

        return static_cast<std::int64_t>(stream_->read(destination));
    });
}

arrow::Result<std::shared_ptr<arrow::Buffer>> Arrow_file::Read(std::int64_t nbytes) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::shared_ptr<arrow::Buffer>>([=]() {
        auto size = static_cast<std::size_t>(nbytes);

        return std::make_shared<Arrow_buffer>(stream_->read(size));
    });
}

arrow::Status Arrow_file::Seek(std::int64_t position) noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary([=]() {
        stream_->seek(static_cast<std::size_t>(position));
    });
}

arrow::Status Arrow_file::Close() noexcept
{
    stream_->close();

    return arrow::Status::OK();
}

arrow::Result<std::int64_t> Arrow_file::Tell() const noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::int64_t>([=]() {
        return static_cast<std::int64_t>(stream_->position());
    });
}

arrow::Result<std::int64_t> Arrow_file::GetSize() noexcept
{
    RETURN_NOT_OK(check_if_closed());

    return arrow_boundary<std::int64_t>([=]() {
        return static_cast<std::int64_t>(stream_->size());
    });
}

arrow::Status Arrow_file::check_if_closed() const noexcept
{
    if (stream_->closed()) {
        return arrow::Status::Invalid("Invalid operation on closed File.");
    }
    return arrow::Status::OK();
}

bool Arrow_file::supports_zero_copy() const noexcept
{
    return stream_->supports_zero_copy();
}

bool Arrow_file::closed() const noexcept
{
    return stream_->closed();
}

}  // namespace pymlio
