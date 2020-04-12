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
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/record_readers/text_line_record_reader.h"
#include "mlio/streams/utf8_input_stream.h"

using mlio::detail::text_line_record_reader;

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
    auto strm = make_utf8_stream(ds.open_read());
    return make_intrusive<text_line_record_reader>(std::move(strm), false);
}

intrusive_ptr<schema const>
text_line_reader::infer_schema(std::optional<instance> const &)
{
    std::vector<attribute> attrs{
        attribute{"value", data_type::string, {params().batch_size, 1}}};

    return make_intrusive<schema>(std::move(attrs));
}

intrusive_ptr<example>
text_line_reader::decode(instance_batch const &batch) const
{
    std::vector<std::string> strs{};
    strs.reserve(batch.instances().size());

    for (instance const &ins : batch.instances()) {
        auto tmp = as_span<char const>(ins.bits());
        strs.emplace_back(tmp.data(), 0, tmp.size());
    }

    size_vector shp{batch.size(), 1};

    auto arr = wrap_cpu_array<data_type::string>(std::move(strs));

    std::vector<intrusive_ptr<tensor>> tensors{
        make_intrusive<dense_tensor>(std::move(shp), std::move(arr))};

    return make_intrusive<example>(get_schema(), std::move(tensors));
}

}  // namespace v1
}  // namespace mlio
