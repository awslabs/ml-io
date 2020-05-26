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
#include "mlio/fwd.h"

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup tensors Tensors
/// @{

/// Provides an implementation of the GoF visitor pattern for performing
/// operations on Tensor classes.
class MLIO_API Tensor_visitor {
public:
    Tensor_visitor() noexcept = default;

    Tensor_visitor(const Tensor_visitor &) = delete;

    Tensor_visitor &operator=(const Tensor_visitor &) = delete;

    Tensor_visitor(Tensor_visitor &&) = delete;

    Tensor_visitor &operator=(Tensor_visitor &&) = delete;

    virtual ~Tensor_visitor();

    virtual void visit(Tensor &tensor);

    virtual void visit(const Tensor &tensor);

    virtual void visit(Dense_tensor &tensor);

    virtual void visit(const Dense_tensor &tensor);

    virtual void visit(Coo_tensor &tensor);

    virtual void visit(const Coo_tensor &tensor);

    virtual void visit(Csr_tensor &tensor);

    virtual void visit(const Csr_tensor &tensor);
};

// @}

}  // namespace abi_v1
}  // namespace mlio
