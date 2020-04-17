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

using mlio::detail::text_line_record_reader;

namespace mlio {
inline namespace v1 {

text_line_reader::text_line_reader(data_reader_params prm)
    : parallel_data_reader{std::move(prm)}
{}

text_line_reader::~text_line_reader()
{
    stop();
}

intrusive_ptr<record_reader>
text_line_reader::make_record_reader(data_store const &ds)
{
    auto strm = make_utf8_stream(ds.open_read());
    return make_intrusive<text_line_record_reader>(std::move(strm), false);
}

intrusive_ptr<schema const>
text_line_reader::infer_schema(std::optional<instance> const &)
{
    std::vector<attribute> attrs{};
    attrs.emplace_back(
        "value", data_type::string, size_vector{params().batch_size, 1});

    return make_intrusive<schema>(std::move(attrs));
}

intrusive_ptr<example>
text_line_reader::decode(instance_batch const &batch) const
{
    intrusive_ptr<dense_tensor> tsr = make_tensor(batch.size());

    auto row_pos = tsr->data().as<std::string>().begin();

    for (instance const &ins : batch.instances()) {
        *row_pos++ = as_string_view(ins.bits());
    }

    std::vector<intrusive_ptr<tensor>> tensors{};
    tensors.emplace_back(std::move(tsr));

    auto exm = make_intrusive<example>(get_schema(), std::move(tensors));

    exm->padding = batch.size() - batch.instances().size();

    return exm;
}

intrusive_ptr<dense_tensor>
text_line_reader::make_tensor(std::size_t batch_size)
{
    size_vector shp{batch_size, 1};

    auto arr = make_cpu_array(data_type::string, batch_size);

    return make_intrusive<dense_tensor>(std::move(shp), std::move(arr));
}

}  // namespace v1
}  // namespace mlio
