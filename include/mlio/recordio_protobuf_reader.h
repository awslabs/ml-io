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

#pragma once

#include "mlio/config.h"
#include "mlio/data_type.h"
#include "mlio/fwd.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/schema.h"

namespace aialgs {
namespace data {

class Record;
class Value;

}  // namespace data
}  // namespace aialgs

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Represents a @ref data_reader for reading Amazon SageMaker
/// RecordIO-protobuf datasets.
class MLIO_API recordio_protobuf_reader final : public parallel_data_reader {
    class decoder;
    class decoder_state;

public:
    explicit
    recordio_protobuf_reader(data_reader_params prm);

    recordio_protobuf_reader(recordio_protobuf_reader const &) = delete;

    recordio_protobuf_reader(recordio_protobuf_reader &&) = delete;

   ~recordio_protobuf_reader() final;

public:
    recordio_protobuf_reader &
    operator=(recordio_protobuf_reader const &) = delete;

    recordio_protobuf_reader &
    operator=(recordio_protobuf_reader &&) = delete;

private:
    MLIO_HIDDEN
    intrusive_ptr<record_reader>
    make_record_reader(data_store const &ds) final;

    MLIO_HIDDEN
    void
    infer_schema(instance const &ins) final;

    MLIO_HIDDEN
    feature_desc
    make_feature_desc(instance const &ins, std::string const &name,
                      aialgs::data::Value const &value);

    template<data_type dt, typename ProtobufTensor>
    MLIO_HIDDEN
    feature_desc
    make_feature_desc(instance const &ins, std::string const &name,
                      ProtobufTensor const &tsr);

    template<typename ProtobufTensor>
    MLIO_HIDDEN
    void
    copy_shape(instance const &ins, std::string const &name, size_vector &shp,
               ProtobufTensor const &tsr);

    MLIO_HIDDEN
    intrusive_ptr<example>
    decode(instance_batch const &batch) const final;

    MLIO_HIDDEN
    static aialgs::data::Record const *
    parse_proto(instance const &ins);

private:
    intrusive_ptr<schema> schema_{};
    bool has_sparse_feature_{};
    std::size_t num_values_per_instance_{};
};

/// @}

}  // namespace v1
}  // namespace mlio
