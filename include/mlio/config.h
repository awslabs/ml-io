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

#if defined(__linux__)
#   define MLIO_PLATFORM_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#   define MLIO_PLATFORM_MACOS
#elif defined(_WIN32)
#   define MLIO_PLATFORM_WIN32
#else
#   error "Unsupported platform!"
#endif

#if defined(MLIO_PLATFORM_LINUX) || defined(MLIO_PLATFORM_MACOS)
#   define MLIO_PLATFORM_POSIX
#endif

#ifdef MLIO_STATIC_LIB
#define MLIO_API
#define MLIO_HIDDEN
#else
#   ifdef _WIN32
#       ifdef MLIO_COMPILE_LIB
#           define MLIO_API __declspec(dllexport)
#       else
#           define MLIO_API __declspec(dllimport)
#       endif
#       define MLIO_HIDDEN
#   else
#       define MLIO_API    __attribute__((visibility("default")))
#       define MLIO_HIDDEN __attribute__((visibility("hidden")))
#   endif
#endif

#if __cplusplus >= 201703L
#   define MLIO_FALLTHROUGH [[fallthrough]]
#elif defined(__clang__)
#   define MLIO_FALLTHROUGH [[clang::fallthrough]]
#elif defined(__GNUC__) && __GNUC__ >= 7
#   define MLIO_FALLTHROUGH [[gnu::fallthrough]]
#else
#   define MLIO_FALLTHROUGH ((void) 0)
#endif
