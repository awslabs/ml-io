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

#include "mlio/config.h"
#include "mlio/fwd.h"

namespace mlio {
inline namespace v1 {

/// @addtogroup tensors Tensors
/// @{

/// Provides an implementation of the GoF visitor pattern for performing
/// operations on tensor classes.
class MLIO_API tensor_visitor {
public:
    tensor_visitor() noexcept = default;

    tensor_visitor(tensor_visitor const &) = delete;

    tensor_visitor(tensor_visitor &&) = delete;

    virtual
   ~tensor_visitor();

public:
    tensor_visitor &
    operator=(tensor_visitor const &) = delete;

    tensor_visitor &
    operator=(tensor_visitor &&) = delete;

public:
    virtual void
    visit(tensor &tsr);

    virtual void
    visit(tensor const &tsr);

    virtual void
    visit(dense_tensor &tsr);

    virtual void
    visit(dense_tensor const &tsr);

    virtual void
    visit(coo_tensor &tsr);

    virtual void
    visit(coo_tensor const &tsr);

    virtual void
    visit(csr_tensor &tsr);

    virtual void
    visit(csr_tensor const &tsr);
};

// @}

}  // namespace v1
}  // namespace mlio
