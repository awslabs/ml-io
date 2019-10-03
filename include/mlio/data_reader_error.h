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

#include <stdexcept>

#include "mlio/config.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

class MLIO_API data_reader_error : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;

public:
    data_reader_error(data_reader_error const &) = default;

    data_reader_error(data_reader_error &&) = default;

   ~data_reader_error() override;

public:
    data_reader_error &
    operator=(data_reader_error const &) = default;

    data_reader_error &
    operator=(data_reader_error &&) = default;
};

class MLIO_API schema_error : public data_reader_error {
public:
    using data_reader_error::data_reader_error;

public:
    schema_error(schema_error const &) = default;

    schema_error(schema_error &&) = default;

   ~schema_error() override;

public:
    schema_error &
    operator=(schema_error const &) = default;

    schema_error &
    operator=(schema_error &&) = default;
};

class MLIO_API invalid_instance_error : public data_reader_error {
public:
    using data_reader_error::data_reader_error;

public:
    invalid_instance_error(invalid_instance_error const &) = default;

    invalid_instance_error(invalid_instance_error &&) = default;

   ~invalid_instance_error() override;

public:
    invalid_instance_error &
    operator=(invalid_instance_error const &) = default;

    invalid_instance_error &
    operator=(invalid_instance_error &&) = default;
};

/// @}

}  // namespace v1
}  // namespace mlio
