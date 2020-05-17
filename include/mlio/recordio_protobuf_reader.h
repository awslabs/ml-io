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

namespace aialgs {
namespace data {

class Record;
class Value;

}  // namespace data
}  // namespace aialgs

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a @ref data_reader for reading Amazon SageMaker
/// RecordIO-protobuf datasets.
class MLIO_API recordio_protobuf_reader final : public parallel_data_reader {
    class decoder_state;
    class decoder;

public:
    explicit recordio_protobuf_reader(data_reader_params prm);

    recordio_protobuf_reader(const recordio_protobuf_reader &) = delete;

    recordio_protobuf_reader(recordio_protobuf_reader &&) = delete;

    ~recordio_protobuf_reader() final;

public:
    recordio_protobuf_reader &operator=(const recordio_protobuf_reader &) = delete;

    recordio_protobuf_reader &operator=(recordio_protobuf_reader &&) = delete;

private:
    MLIO_HIDDEN
    intrusive_ptr<record_reader> make_record_reader(const data_store &ds) final;

    MLIO_HIDDEN
    intrusive_ptr<schema const> infer_schema(std::optional<instance> const &ins) final;

    MLIO_HIDDEN
    attribute
    make_attribute(const instance &ins, const std::string &name, const aialgs::data::Value &value);

    template<data_type dt, typename ProtobufTensor>
    MLIO_HIDDEN
    attribute
    make_attribute(const instance &ins, const std::string &name, const ProtobufTensor &tsr);

    template<typename ProtobufTensor>
    MLIO_HIDDEN
    void copy_shape(const instance &ins,
                    const std::string &name,
                    size_vector &shp,
                    const ProtobufTensor &tsr);

    MLIO_HIDDEN
    intrusive_ptr<example> decode(const instance_batch &batch) const final;

    MLIO_HIDDEN
    std::optional<std::size_t> decode_ser(decoder_state &state, const instance_batch &batch) const;

    MLIO_HIDDEN
    std::optional<std::size_t> decode_prl(decoder_state &state, const instance_batch &batch) const;

    MLIO_HIDDEN
    static const aialgs::data::Record *parse_proto(const instance &ins);

private:
    bool has_sparse_feature_{};
    std::size_t num_values_per_instance_{};
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
