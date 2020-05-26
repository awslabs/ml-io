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
#include "mlio/data_reader.h"
#include "mlio/example.h"
#include "mlio/intrusive_ptr.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents an abstract base class for data readers.
class MLIO_API Data_reader_base : public Data_reader {
public:
    Intrusive_ptr<Example> read_example() final;

    Intrusive_ptr<Example> peek_example() final;

    void reset() noexcept override;

    const Data_reader_params &params() const noexcept
    {
        return params_;
    }

protected:
    explicit Data_reader_base(Data_reader_params &&params) noexcept;

    /// Gets the effective boolean value indicating whether a warning
    /// will be output for each bad instance.
    bool warn_bad_instances() const noexcept
    {
        return warn_bad_instances_;
    }

private:
    /// When implemented in a derived class, returns the next @ref
    /// Example read from the dataset.
    virtual Intrusive_ptr<Example> read_example_core() = 0;

    Data_reader_params params_;
    bool warn_bad_instances_{};
    Intrusive_ptr<Example> peeked_example_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
