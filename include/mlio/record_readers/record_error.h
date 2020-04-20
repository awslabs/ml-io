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

#include "mlio/config.h"
#include "mlio/mlio_error.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup records Records
/// @{

class MLIO_API record_error : public mlio_error {
public:
    using mlio_error::mlio_error;

public:
    record_error(record_error const &) = default;

    record_error(record_error &&) = default;

    ~record_error() override;

public:
    record_error &operator=(record_error const &) = default;

    record_error &operator=(record_error &&) = default;
};

class MLIO_API corrupt_record_error : public record_error {
public:
    using record_error::record_error;

public:
    corrupt_record_error(corrupt_record_error const &) = default;

    corrupt_record_error(corrupt_record_error &&) = default;

    ~corrupt_record_error() override;

public:
    corrupt_record_error &operator=(corrupt_record_error const &) = default;

    corrupt_record_error &operator=(corrupt_record_error &&) = default;
};

class MLIO_API corrupt_header_error : public corrupt_record_error {
public:
    using corrupt_record_error::corrupt_record_error;

public:
    corrupt_header_error(corrupt_header_error const &) = default;

    corrupt_header_error(corrupt_header_error &&) = default;

    ~corrupt_header_error() override;

public:
    corrupt_header_error &operator=(corrupt_header_error const &) = default;

    corrupt_header_error &operator=(corrupt_header_error &&) = default;
};

class MLIO_API corrupt_footer_error : public corrupt_record_error {
public:
    using corrupt_record_error::corrupt_record_error;

public:
    corrupt_footer_error(corrupt_footer_error const &) = default;

    corrupt_footer_error(corrupt_footer_error &&) = default;

    ~corrupt_footer_error() override;

public:
    corrupt_footer_error &operator=(corrupt_footer_error const &) = default;

    corrupt_footer_error &operator=(corrupt_footer_error &&) = default;
};

class MLIO_API record_too_large_error : public record_error {
public:
    using record_error::record_error;

public:
    record_too_large_error(record_too_large_error const &) = default;

    record_too_large_error(record_too_large_error &&) = default;

    ~record_too_large_error() override;

public:
    record_too_large_error &operator=(record_too_large_error const &) = default;

    record_too_large_error &operator=(record_too_large_error &&) = default;
};

/// @}

}  // namespace v1
}  // namespace mlio
