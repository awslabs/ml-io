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

#include "mlio/init.h"

#ifdef MLIO_BUILD_S3

#include <aws/core/Aws.h>

#include "mlio/logger.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

bool aws_initialized{};

}  // namespace
}  // namespace detail

void initialize_aws_sdk()
{
    if (detail::aws_initialized) {
        logger::warn("AWS C++ SDK is already initialized.");

        return;
    }

    Aws::InitAPI({});

    detail::aws_initialized = true;
}

void deallocate_aws_sdk()
{
    if (detail::aws_initialized) {
        Aws::ShutdownAPI({});
    }

    detail::aws_initialized = false;
}

}  // namespace abi_v1
}  // namespace mlio

#else

#include "mlio/not_supported_error.h"

namespace mlio {
inline namespace abi_v1 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"

void initialize_aws_sdk()
{
    throw Not_supported_error{"MLIO was not built with S3 support."};
}

void deallocate_aws_sdk()
{
    throw Not_supported_error{"MLIO was not built with S3 support."};
}

#pragma GCC diagnostic pop

}  // namespace abi_v1
}  // namespace mlio

#endif
