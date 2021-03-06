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

#include <exception>
#include <new>
#include <stdexcept>
#include <utility>

#include <arrow/result.h>
#include <arrow/status.h>

#include <mlio/not_supported_error.h>
#include <mlio/streams/stream_error.h>

namespace pymlio {
namespace detail {

inline arrow::Status convert_exception()
{
    try {
        throw;
    }
    catch (const std::bad_alloc &e) {
        return arrow::Status::OutOfMemory(e.what());
    }
    catch (const std::invalid_argument &e) {
        return arrow::Status::Invalid(e.what());
    }
    catch (const mlio::Stream_error &e) {
        return arrow::Status::IOError(e.what());
    }
    catch (const mlio::Not_supported_error &e) {
        return arrow::Status::NotImplemented(e.what());
    }
    catch (const std::exception &e) {
        return arrow::Status::UnknownError(e.what());
    }
    catch (...) {
        return arrow::Status::UnknownError("An unknown error has occured.");
    }
}

}  // namespace detail

template<typename Function, typename... Args>
arrow::Status arrow_boundary(Function &&f, Args &&... args) noexcept
{
    try {
        std::forward<Function>(f)(std::forward<Args>(args)...);
    }
    catch (...) {
        return detail::convert_exception();
    }

    return arrow::Status::OK();
}

template<typename T, typename Function, typename... Args>
arrow::Result<T> arrow_boundary(Function &&f, Args &&... args) noexcept
{
    try {
        return std::forward<Function>(f)(std::forward<Args>(args)...);
    }
    catch (...) {
        return detail::convert_exception();
    }
}

}  // namespace pymlio
