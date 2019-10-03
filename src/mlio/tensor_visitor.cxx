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

#include "mlio/tensor_visitor.h"

#include "mlio/not_supported_error.h"
#include "mlio/tensor.h"

namespace mlio {
inline namespace v1 {

tensor_visitor::~tensor_visitor() = default;

void
tensor_visitor::
visit(tensor &)
{
    throw not_supported_error{
        "The operation is not supported for the specified tensor."};
}

void
tensor_visitor::
visit(tensor const &)
{
    throw not_supported_error{
        "The operation is not supported for the specified tensor."};
}

void
tensor_visitor::
visit(dense_tensor &tsr)
{
    visit(static_cast<tensor &>(tsr));
}

void
tensor_visitor::
visit(dense_tensor const &tsr)
{
    visit(static_cast<tensor const &>(tsr));
}

void
tensor_visitor::
visit(coo_tensor &tsr)
{
    visit(static_cast<tensor &>(tsr));
}

void
tensor_visitor::
visit(coo_tensor const &tsr)
{
    visit(static_cast<tensor const &>(tsr));
}

void
tensor_visitor::
visit(csr_tensor &tsr)
{
    visit(static_cast<tensor &>(tsr));
}

void
tensor_visitor::
visit(csr_tensor const &tsr)
{
    visit(static_cast<tensor const &>(tsr));
}

}  // namespace v1
}  // namespace mlio
