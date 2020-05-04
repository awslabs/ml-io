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

#if defined(__linux__)
#define MLIO_PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define MLIO_PLATFORM_MACOS
#else
#error "Unsupported platform!"
#endif

#ifdef MLIO_STATIC_LIB
#define MLIO_API
#define MLIO_HIDDEN
#else
#define MLIO_API __attribute__((visibility("default")))
#define MLIO_HIDDEN __attribute__((visibility("hidden")))
#endif

namespace mlio {
inline namespace v1 {

/// Returns a boolean value indicating whether the library was built
/// with Amazon S3 support.
MLIO_API
bool supports_s3() noexcept;

/// Returns a boolean value indicating whether the library was built
/// with image reader support.
MLIO_API
bool supports_image_reader() noexcept;

}  // namespace v1
}  // namespace mlio
