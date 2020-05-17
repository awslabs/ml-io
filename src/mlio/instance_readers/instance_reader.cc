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

instance_reader::~instance_reader() = default;

std::unique_ptr<instance_reader>
make_instance_reader(const data_reader_params &prm, record_reader_factory &&fct)
{
    std::unique_ptr<instance_reader> rdr{};

    rdr = std::make_unique<core_instance_reader>(prm, std::move(fct));

    if (prm.num_instances_to_skip > 0 || prm.num_instances_to_read) {
        rdr = std::make_unique<ranged_instance_reader>(prm, std::move(rdr));
    }

    if (prm.num_shards > 1) {
        rdr = std::make_unique<sharded_instance_reader>(prm, std::move(rdr));
    }

    if (prm.sample_ratio) {
        rdr = std::make_unique<sampled_instance_reader>(prm, std::move(rdr));
    }

    if (prm.shuffle_instances) {
        rdr = std::make_unique<shuffled_instance_reader>(prm, std::move(rdr));
    }

    return rdr;
}

}  // namespace detail
}  // namespace abi_v1
}  // namespace mlio
