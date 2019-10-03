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

#include "core/module.h"

namespace mliopy {

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-prototypes"

PYBIND11_MODULE(core, m) {
    m.doc() = "BALIOGLU";

    mlio::initialize();

    register_exceptions(m);
    register_logging(m);
    register_memory_slice(m);
    register_device_array(m);
    register_tensors(m);
    register_integ(m);
    register_streams(m);
    register_data_stores(m);
    register_record_readers(m);
    register_schema(m);
    register_example(m);
    register_data_readers(m);
}

#pragma clang diagnostic pop

}  // namespace mliopy
