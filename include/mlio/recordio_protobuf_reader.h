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

#pragma once

#include <optional>

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/parallel_data_reader.h"

namespace aialgs::data {

class Record;
class Value;

}  // namespace aialgs::data

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a @ref Data_reader for reading Amazon SageMaker
/// RecordIO-protobuf datasets.
class MLIO_API Recordio_protobuf_reader final : public Parallel_data_reader {
public:
    explicit Recordio_protobuf_reader(Data_reader_params params);

    Recordio_protobuf_reader(const Recordio_protobuf_reader &) = delete;

    Recordio_protobuf_reader &operator=(const Recordio_protobuf_reader &) = delete;

    Recordio_protobuf_reader(Recordio_protobuf_reader &&) = delete;

    Recordio_protobuf_reader &operator=(Recordio_protobuf_reader &&) = delete;

    ~Recordio_protobuf_reader() final;

private:
    class Decoder_state;
    class Decoder;

    MLIO_HIDDEN
    Intrusive_ptr<Record_reader> make_record_reader(const Data_store &store) final;

    MLIO_HIDDEN
    Intrusive_ptr<const Schema> infer_schema(const std::optional<Instance> &instance) final;

    MLIO_HIDDEN
    Attribute make_attribute(const Instance &instance,
                             const std::string &name,
                             const aialgs::data::Value &value);

    template<Data_type dt, typename Protobuf_tensor>
    MLIO_HIDDEN
    Attribute make_attribute(const Instance &instance,
                             const std::string &name,
                             const Protobuf_tensor &tensor);

    template<typename Protobuf_tensor>
    MLIO_HIDDEN
    void copy_shape(const Instance &instance,
                    const std::string &name,
                    Size_vector &shape,
                    const Protobuf_tensor &tensor);

    MLIO_HIDDEN
    Intrusive_ptr<Example> decode(const Instance_batch &batch) const final;

    MLIO_HIDDEN
    std::optional<std::size_t>
    decode_serial(Decoder_state &state, const Instance_batch &batch) const;

    MLIO_HIDDEN
    std::optional<std::size_t>
    decode_parallel(Decoder_state &state, const Instance_batch &batch) const;

    MLIO_HIDDEN
    static const aialgs::data::Record *parse_proto(const Instance &instance);

    bool has_sparse_feature_{};
    std::size_t num_values_per_instance_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
