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

#include "module.h"

#include <string>

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

Intrusive_ptr<S3_client> py_make_s3_client(const std::string &access_key_id,
                                           const std::string &secret_key,
                                           const std::string &session_token,
                                           const std::string &profile,
                                           const std::string &region,
                                           bool use_https)
{
    S3_client_options opts{access_key_id, secret_key, session_token, profile, region, use_https};
    return make_s3_client(opts);
}

}  // namespace

void register_s3_client(py::module &m)
{
    py::class_<S3_client, Intrusive_ptr<S3_client>>(
        m, "S3Client", "Represents a client to access Amazon S3.")
        .def(py::init<>(&py_make_s3_client),
             "access_key_id"_a = "",
             "secret_key"_a = "",
             "session_token"_a = "",
             "profile"_a = "",
             "region"_a = "",
             "use_https"_a = true);

    m.def("initialize_aws_sdk", initialize_aws_sdk, "Initialize AWS C++ SDK");
    m.def("deallocate_aws_sdk",
          deallocate_aws_sdk,
          "Deallocate the internal data structures used by AWS C++ SDK.");
}

}  // namespace pymlio
