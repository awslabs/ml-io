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

#include "mlio/instance_readers/instance_reader.h"

#include "mlio/data_reader.h"
#include "mlio/instance_readers/core_instance_reader.h"
#include "mlio/instance_readers/ranged_instance_reader.h"
#include "mlio/instance_readers/sampled_instance_reader.h"
#include "mlio/instance_readers/sharded_instance_reader.h"
#include "mlio/instance_readers/shuffled_instance_reader.h"

namespace mlio {
inline namespace abi_v1 {
namespace detail {

Instance_reader::~Instance_reader() = default;

std::unique_ptr<Instance_reader>
make_instance_reader(const Data_reader_params &params, Record_reader_factory &&factory)
{
    std::unique_ptr<Instance_reader> reader{};

    reader = std::make_unique<Core_instance_reader>(params, std::move(factory));

    if (params.num_instances_to_skip > 0 || params.num_instances_to_read) {
        reader = std::make_unique<Ranged_instance_reader>(params, std::move(reader));
    }

    if (params.num_shards > 1) {
        reader = std::make_unique<Sharded_instance_reader>(params, std::move(reader));
    }

    if (params.sample_ratio) {
        reader = std::make_unique<Sampled_instance_reader>(params, std::move(reader));
    }

    if (params.shuffle_instances) {
        reader = std::make_unique<Shuffled_instance_reader>(params, std::move(reader));
    }

    return reader;
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
