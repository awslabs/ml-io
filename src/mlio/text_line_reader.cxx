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

#include <cstddef>
#include <stdexcept>
#include <vector>

#include "mlio/cpu_array.h"
#include "mlio/data_type.h"
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/record_readers/text_line_record_reader.h"
#include "mlio/schema.h"
#include "mlio/streams/utf8_input_stream.h"
#include "mlio/text_encoding.h"

namespace mlio {
inline namespace v1 {

text_line_reader::text_line_reader(data_reader_params rdr_prm)
    : parallel_data_reader{std::move(rdr_prm)}
{}

text_line_reader::~text_line_reader()
{
    stop();
}

intrusive_ptr<record_reader>
text_line_reader::make_record_reader(data_store const &ds)
{
    auto strm = make_utf8_stream(ds.open_read(), text_encoding::utf8);
    auto rdr = make_intrusive<detail::text_line_record_reader>(std::move(strm),
                                                               false);
    return std::move(rdr);
}

void
text_line_reader::infer_schema(instance const &)
{
    std::vector<feature_desc> descs{};
    descs.emplace_back(feature_desc_builder{
        "value", data_type::string, {params().batch_size, 1}}
                           .build());
    schema_ = make_intrusive<schema>(std::move(descs));
}

intrusive_ptr<example>
text_line_reader::decode(instance_batch const &batch) const
{
    std::vector<instance> const &instances = batch.instances();
    auto tsr = make_tensor(instances, batch.size());
    std::vector<intrusive_ptr<tensor>> tensors{};
    tensors.emplace_back(std::move(tsr));
    return make_intrusive<example>(schema_, std::move(tensors));
}

intrusive_ptr<dense_tensor>
text_line_reader::make_tensor(std::vector<instance> const &instances,
                              std::size_t batch_size)
{
    std::vector<std::string> strings{};
    for (const instance &ins : instances) {
        auto temp = as_span<char const>(ins.bits());
        std::string text_line(temp.data(), 0, temp.size());
        strings.emplace_back(std::move(text_line));
    }
    auto arr = wrap_cpu_array<data_type::string>(std::move(strings));
    auto tsr = make_intrusive<dense_tensor>(size_vector{batch_size, 1},
                                            std::move(arr));
    return tsr;
}

}  // namespace v1
}  // namespace mlio
