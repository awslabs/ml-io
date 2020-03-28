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

#include "module.h"

#include <string>

namespace py = pybind11;

using namespace mlio;
using namespace pybind11::literals;

namespace pymlio {
namespace {

intrusive_ptr<s3_client>
py_make_s3_client(std::string access_key_id,
                  std::string secret_key,
                  std::string session_token,
                  std::string profile,
                  std::string region,
                  bool use_https)
{
    return s3_client_builder{}
        .with_access_key_id(std::move(access_key_id))
        .with_secret_key(std::move(secret_key))
        .with_session_token(std::move(session_token))
        .with_profile(std::move(profile))
        .with_region(std::move(region))
        .with_https(use_https)
        .build();
}

}  // namespace

void
register_s3_client(py::module &m)
{
    py::class_<s3_client, intrusive_ptr<s3_client>>(
        m, "S3Client", "Represents a client to access Amazon S3.")
        .def(py::init<>(&py_make_s3_client),
             "access_key_id"_a = "",
             "secret_key"_a = "",
             "session_token"_a = "",
             "profile"_a = "",
             "region"_a = "",
             "use_https"_a = true);

    m.def("initialize_aws_sdk",
          initialize_aws_sdk,
          "Initialize AWS C++ SDK");
    m.def("dispose_aws_sdk",
          dispose_aws_sdk,
          "Dispose the internal data structures used by AWS C++ SDK.");
}

}  // namespace pymlio
