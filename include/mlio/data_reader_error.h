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

/// @addtogroup data_readers Data Readers
/// @{

class MLIO_API Data_reader_error : public Mlio_error {
public:
    using Mlio_error::Mlio_error;

    Data_reader_error(const Data_reader_error &) = default;

    Data_reader_error &operator=(const Data_reader_error &) = default;

    Data_reader_error(Data_reader_error &&) = default;

    Data_reader_error &operator=(Data_reader_error &&) = default;

    ~Data_reader_error() override;
};

class MLIO_API Schema_error : public Data_reader_error {
public:
    using Data_reader_error::Data_reader_error;

    Schema_error(const Schema_error &) = default;

    Schema_error &operator=(const Schema_error &) = default;

    Schema_error(Schema_error &&) = default;

    Schema_error &operator=(Schema_error &&) = default;

    ~Schema_error() override;
};

class MLIO_API Invalid_instance_error : public Data_reader_error {
public:
    using Data_reader_error::Data_reader_error;

    Invalid_instance_error(const Invalid_instance_error &) = default;

    Invalid_instance_error &operator=(const Invalid_instance_error &) = default;

    Invalid_instance_error(Invalid_instance_error &&) = default;

    Invalid_instance_error &operator=(Invalid_instance_error &&) = default;

    ~Invalid_instance_error() override;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
