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
inline namespace abi_v1 {

/// @addtogroup records Records
/// @{

class MLIO_API Record_error : public Mlio_error {
public:
    using Mlio_error::Mlio_error;

    Record_error(const Record_error &) = default;

    Record_error &operator=(const Record_error &) = default;

    Record_error(Record_error &&) = default;

    Record_error &operator=(Record_error &&) = default;

    ~Record_error() override;
};

class MLIO_API Corrupt_record_error : public Record_error {
public:
    using Record_error::Record_error;

    Corrupt_record_error(const Corrupt_record_error &) = default;

    Corrupt_record_error &operator=(const Corrupt_record_error &) = default;

    Corrupt_record_error(Corrupt_record_error &&) = default;

    Corrupt_record_error &operator=(Corrupt_record_error &&) = default;

    ~Corrupt_record_error() override;
};

class MLIO_API Corrupt_header_error : public Corrupt_record_error {
public:
    using Corrupt_record_error::Corrupt_record_error;

    Corrupt_header_error(const Corrupt_header_error &) = default;

    Corrupt_header_error &operator=(const Corrupt_header_error &) = default;

    Corrupt_header_error(Corrupt_header_error &&) = default;

    Corrupt_header_error &operator=(Corrupt_header_error &&) = default;

    ~Corrupt_header_error() override;
};

class MLIO_API Corrupt_footer_error : public Corrupt_record_error {
public:
    using Corrupt_record_error::Corrupt_record_error;

    Corrupt_footer_error(const Corrupt_footer_error &) = default;

    Corrupt_footer_error &operator=(const Corrupt_footer_error &) = default;

    Corrupt_footer_error(Corrupt_footer_error &&) = default;

    Corrupt_footer_error &operator=(Corrupt_footer_error &&) = default;

    ~Corrupt_footer_error() override;
};

class MLIO_API Record_too_large_error : public Record_error {
public:
    using Record_error::Record_error;

    Record_too_large_error(const Record_too_large_error &) = default;

    Record_too_large_error &operator=(const Record_too_large_error &) = default;

    Record_too_large_error(Record_too_large_error &&) = default;

    Record_too_large_error &operator=(Record_too_large_error &&) = default;

    ~Record_too_large_error() override;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
