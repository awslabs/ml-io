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

#include "mlio/text_line_reader.h"

#include <string>

#include "mlio/cpu_array.h"
#include "mlio/data_type.h"
#include "mlio/example.h"
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/record_readers/text_line_record_reader.h"
#include "mlio/streams/utf8_input_stream.h"
#include "mlio/tensor.h"
#include "mlio/util/string.h"

using mlio::detail::Text_line_record_reader;

namespace mlio {
inline namespace abi_v1 {

Text_line_reader::Text_line_reader(Data_reader_params params)
    : Parallel_data_reader{std::move(params)}
{}

Text_line_reader::~Text_line_reader()
{
    stop();
}

Intrusive_ptr<Record_reader> Text_line_reader::make_record_reader(const Data_store &store)
{
    auto stream = make_utf8_stream(store.open_read());
    return make_intrusive<Text_line_record_reader>(std::move(stream), false);
}

Intrusive_ptr<const Schema> Text_line_reader::infer_schema(const std::optional<Instance> &)
{
    std::vector<Attribute> attrs{};
    attrs.emplace_back("value", Data_type::string, Size_vector{params().batch_size, 1});

    return make_intrusive<Schema>(std::move(attrs));
}

Intrusive_ptr<Example> Text_line_reader::decode(const Instance_batch &batch) const
{
    Intrusive_ptr<Dense_tensor> tensor = make_tensor(batch.size());

    auto row_pos = tensor->data().as<std::string>().begin();
    for (const Instance &instance : batch.instances()) {
        *row_pos++ = as_string_view(instance.bits());
    }

    std::vector<Intrusive_ptr<Tensor>> tensors{};
    tensors.emplace_back(std::move(tensor));

    auto example = make_intrusive<Example>(schema(), std::move(tensors));

    example->padding = batch.size() - batch.instances().size();

    return example;
}

Intrusive_ptr<Dense_tensor> Text_line_reader::make_tensor(std::size_t batch_size)
{
    Size_vector shape{batch_size, 1};

    auto arr = make_cpu_array(Data_type::string, batch_size);

    return make_intrusive<Dense_tensor>(std::move(shape), std::move(arr));
}

}  // namespace abi_v1
}  // namespace mlio
