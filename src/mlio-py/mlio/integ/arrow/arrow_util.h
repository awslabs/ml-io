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

#include <exception>
#include <new>
#include <stdexcept>
#include <utility>

#include <arrow/status.h>

#include <mlio.h>

namespace mliopy {

template<typename Function, typename... Args>
arrow::Status
arrow_boundary(Function &&f, Args &&...args) noexcept
{
    try {
        std::forward<Function>(f)(std::forward<Args>(args)...);
    }
    catch (std::bad_alloc const &e) {
        return arrow::Status::OutOfMemory(e.what());
    }
    catch (std::invalid_argument const &e) {
        return arrow::Status::Invalid(e.what());
    }
    catch (mlio::stream_error const &e) {
        return arrow::Status::IOError(e.what());
    }
    catch (mlio::not_supported_error const &e) {
        return arrow::Status::NotImplemented(e.what());
    }
    catch (std::exception const &e) {
        return arrow::Status::UnknownError(e.what());
    }
    catch (...) {
        return arrow::Status::UnknownError("An unknown error has occured.");
    }

    return arrow::Status::OK();
}

}  // namespace mliopy
