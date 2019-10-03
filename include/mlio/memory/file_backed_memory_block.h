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

#include "mlio/config.h"  // IWYU pragma: keep

#ifdef MLIO_PLATFORM_WIN32
#   include "mlio/platform/win32/memory/file_backed_memory_block.h"  // IWYU pragma: export
#else
#   include "mlio/platform/posix/memory/file_backed_memory_block.h"  // IWYU pragma: export
#endif
