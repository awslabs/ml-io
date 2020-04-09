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

#include "mlio/recordio_protobuf_reader.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <utility>
#include <vector>

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <tbb/tbb.h>

#include "mlio/coo_tensor_builder.h"
#include "mlio/cpu_array.h"
#include "mlio/data_reader_error.h"
#include "mlio/detail/protobuf/recordio_protobuf.pb.h"
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/logger.h"
#include "mlio/not_supported_error.h"
#include "mlio/record_readers/recordio_record_reader.h"
#include "mlio/tensor.h"
#include "mlio/util/cast.h"

using aialgs::data::Record;
using aialgs::data::Value;

using mlio::detail::coo_tensor_builder;
using mlio::detail::coo_tensor_builder_impl;
using mlio::detail::make_coo_tensor_builder;

namespace mlio {
inline namespace v1 {
namespace detail {
namespace {

// Constructing protobuf message objects is an expensive operation;
// therefore we re-use a single instance per thread.
thread_local Record proto_msg_{};  // NOLINT(cert-err58-cpp)

}  // namespace
}  // namespace detail

class recordio_protobuf_reader::decoder_state {
public:
    explicit decoder_state(recordio_protobuf_reader const &reader,
                           std::size_t batch_size);

private:
    void
    init_state(schema const &shm, std::size_t batch_size);

    void
    init_tensor(attribute const &attr, std::size_t batch_size);

    void
    init_coo_tensor_builder(attribute const &attr, std::size_t batch_size);

public:
    std::vector<intrusive_ptr<tensor>> tensors;
    std::vector<std::unique_ptr<coo_tensor_builder>> coo_tensor_builders;
    bad_batch_handling bbh;
};

class recordio_protobuf_reader::decoder {
public:
    explicit decoder(recordio_protobuf_reader const &reader,
                     decoder_state &state)
        : reader_{&reader}, state_{&state}
    {}

public:
    bool
    decode(std::size_t row_idx, instance const &ins);

private:
    Record const *
    parse_proto(instance const &ins) const;

    bool
    decode_feature(std::string const &name, Value const &value);

    template<data_type dt, typename ProtobufTensor>
    bool
    decode_feature(ProtobufTensor const &tsr);

    template<typename ProtobufTensor>
    bool
    is_sparse(ProtobufTensor const &tsr) const;

    template<typename ProtobufTensor>
    bool
    shape_equals(ProtobufTensor const &tsr) const;

    template<data_type dt, typename ProtobufTensor>
    bool
    copy_to_tensor(ProtobufTensor const &tsr) const;

    template<data_type dt, typename ProtobufTensor>
    bool
    append_to_builder(ProtobufTensor const &tsr) const;

private:
    recordio_protobuf_reader const *reader_;
    decoder_state *state_;
    instance const *instance_{};
    std::size_t row_idx_{};
    std::size_t ftr_idx_{};
    attribute const *attr_{};
};

recordio_protobuf_reader::recordio_protobuf_reader(data_reader_params prm)
    : parallel_data_reader{std::move(prm)}
{}

recordio_protobuf_reader::~recordio_protobuf_reader()
{
    // Make sure that we stop parallel reading before the member objects
    // get destructed; otherwise a background task might try to access
    // an already destructed object.
    stop();
}

intrusive_ptr<record_reader>
recordio_protobuf_reader::make_record_reader(data_store const &ds)
{
    return make_intrusive<detail::recordio_record_reader>(ds.open_read());
}

intrusive_ptr<schema const>
recordio_protobuf_reader::infer_schema(std::optional<instance> const &ins)
{
    if (ins == std::nullopt) {
        return {};
    }

    Record const *proto_msg = parse_proto(*ins);
    if (proto_msg == nullptr) {
        throw schema_error{fmt::format(
            "The record {1:n} in the data store {0} contains a corrupt "
            "RecordIO-protobuf message.",
            ins->get_data_store(),
            ins->index() + 1)};
    }

    std::vector<attribute> attrs{};

    for (auto &[label, value] : proto_msg->label()) {
        // The label and feature maps of a RecordIO-protobuf message can
        // contain same-named features. In order to avoid name clashes
        // we use the "label_" prefix for the labels.
        attrs.emplace_back(make_attribute(*ins, "label_" + label, value));
    }
    for (auto &[label, value] : proto_msg->features()) {
        attrs.emplace_back(make_attribute(*ins, label, value));
    }

    auto shm = make_intrusive<schema>(std::move(attrs));

    // We use this value in the decode() function to decide whether the
    // amount of data we need to process is worth to parallelize.
    for (attribute const &attr : shm->attributes()) {
        if (!attr.sparse()) {
            // The stride of the batch dimension.
            num_values_per_instance_ += as_size(attr.strides()[0]);
        }
    }

    return std::move(shm);
}

attribute
recordio_protobuf_reader::make_attribute(instance const &ins,
                                         std::string const &name,
                                         Value const &value)
{
    switch (value.value_case()) {
    case Value::ValueCase::kFloat32Tensor:
        return make_attribute<data_type::float32>(
            ins, name, value.float32_tensor());

    case Value::ValueCase::kFloat64Tensor:
        return make_attribute<data_type::float64>(
            ins, name, value.float64_tensor());

    case Value::ValueCase::kInt32Tensor:
        return make_attribute<data_type::sint32>(
            ins, name, value.int32_tensor());

    case Value::ValueCase::kBytes:
        throw not_supported_error{
            "The RecordIO-protobuf binary data format is not supported."};

    case Value::ValueCase::VALUE_NOT_SET:
        break;
    }

    throw schema_error{fmt::format(
        "The feature '{2}' of the record {1:n} in the data store {0} has an "
        "unsupported data type.",
        ins.get_data_store(),
        ins.index() + 1,
        name)};
}

template<data_type dt, typename ProtobufTensor>
attribute
recordio_protobuf_reader::make_attribute(instance const &ins,
                                         std::string const &name,
                                         ProtobufTensor const &tsr)
{
    bool is_sparse{};

    size_vector shape{params().batch_size};

    if (tsr.keys().empty()) {
        if (tsr.shape().empty()) {
            // If both the shape and the key array are empty, we treat
            // the feature as a dense vector.
            shape.emplace_back(static_cast<std::size_t>(tsr.values_size()));
        }
        else {
            copy_shape(ins, name, shape, tsr);

            // If the feature has neither a key nor a value, there is no
            // way to determine if it is dense or sparse. The common
            // practice is to check whether it has a shape and treat it
            // as sparse in such case.
            if (tsr.values().empty()) {
                is_sparse = true;
            }
        }
    }
    else {
        is_sparse = true;

        if (tsr.shape().empty()) {
            throw schema_error{fmt::format(
                "The sparse feature '{2}' of the record {1:n} in the data "
                "store {0} has no shape specified.",
                ins.get_data_store(),
                ins.index() + 1,
                name)};
        }

        copy_shape(ins, name, shape, tsr);
    }

    if (is_sparse) {
        has_sparse_feature_ = true;
    }

    return attribute_builder{name, dt, std::move(shape)}
        .with_sparsity(is_sparse);
}

template<typename ProtobufTensor>
void
recordio_protobuf_reader::copy_shape(instance const &ins,
                                     std::string const &name,
                                     size_vector &shp,
                                     ProtobufTensor const &tsr)
{
    for (std::uint64_t dim : tsr.shape()) {
        std::size_t d{};
        if (!try_narrow(dim, d)) {
            std::size_t s =
                std::numeric_limits<std::byte>::digits * sizeof(std::size_t);

            throw schema_error{fmt::format(
                "The shape of the feature '{2}' of the record {1:n} in the "
                "data store {0} has a dimension that is larger than {3}-bits.",
                ins.get_data_store(),
                ins.index() + 1,
                name,
                s)};
        }

        shp.emplace_back(d);
    }
}

intrusive_ptr<example>
recordio_protobuf_reader::decode(instance_batch const &batch) const
{
    decoder_state dec_state{*this, batch.size()};

    std::atomic_bool skip_batch{};

    std::size_t num_instances = batch.instances().size();

    auto row_idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto row_idx_end = tbb::counting_iterator<std::size_t>(num_instances);

    auto rec_beg = batch.instances().begin();
    auto rec_end = batch.instances().end();

    auto rng_beg = tbb::make_zip_iterator(row_idx_beg, rec_beg);
    auto rng_end = tbb::make_zip_iterator(row_idx_end, rec_end);

    tbb::blocked_range<decltype(rng_beg)> range{rng_beg, rng_end};

    auto worker = [this, &dec_state, &skip_batch](auto &sub_range) {
        for (auto row_zip : sub_range) {
            decoder dc{*this, dec_state};
            // If we failed to decode the instance, we can terminate the
            // task early and skip this batch.
            if (!dc.decode(std::get<0>(row_zip), std::get<1>(row_zip))) {
                skip_batch = true;

                return;
            }
        }
    };

    constexpr std::size_t cut_off = 10'000'000;

    bool serial =
        // If we have any sparse features, we cannot decode the batch in
        // parallel as we need to append each instance sequentially to
        // the COO tensor.
        has_sparse_feature_ ||
        // If the number of values (e.g. integers, floating-points) we
        // need to decode is below the cut-off, avoid parallel
        // execution; otherwise the threading overhead will slow down
        // the performance.
        num_values_per_instance_ * num_instances < cut_off;

    if (serial) {
        worker(range);
    }
    else {
        tbb::parallel_for(range, worker, tbb::auto_partitioner{});
    }

    // Check if we failed to decode the batch and return a null pointer
    // if that is the case.
    if (skip_batch) {
        return nullptr;
    }

    auto tsr_beg = dec_state.tensors.begin();
    auto tsr_end = dec_state.tensors.end();

    auto bld_beg = dec_state.coo_tensor_builders.begin();
    auto bld_end = dec_state.coo_tensor_builders.end();

    auto ftr_beg = tbb::make_zip_iterator(tsr_beg, bld_beg);
    auto ftr_end = tbb::make_zip_iterator(tsr_end, bld_end);

    for (auto ftr_pos = ftr_beg; ftr_pos < ftr_end; ++ftr_pos) {
        intrusive_ptr<tensor> &tsr = std::get<0>(*ftr_pos);

        // If no tensor exists at the specified location, it means the
        // corresponding feature was sparse. We should build its tensor
        // as we processed the whole batch now.
        if (tsr == nullptr) {
            tsr = std::get<1>(*ftr_pos)->build();
        }
    }

    auto exm =
        make_intrusive<example>(get_schema(), std::move(dec_state.tensors));

    exm->padding = batch.size() - num_instances;

    return exm;
}

Record const *
recordio_protobuf_reader::parse_proto(instance const &ins)
{
    bool parsed = detail::proto_msg_.ParseFromArray(
        ins.bits().data(), static_cast<int>(ins.bits().size()));

    return parsed ? &detail::proto_msg_ : nullptr;
}

recordio_protobuf_reader::decoder_state::decoder_state(
    recordio_protobuf_reader const &reader, std::size_t batch_size)
{
    init_state(*reader.get_schema(), batch_size);

    bbh = reader.effective_bad_batch_handling();
}

void
recordio_protobuf_reader::decoder_state::init_state(schema const &shm,
                                                    std::size_t batch_size)
{
    tensors.reserve(shm.attributes().size());

    coo_tensor_builders.reserve(shm.attributes().size());

    for (attribute const &attr : shm.attributes()) {
        if (attr.sparse()) {
            init_coo_tensor_builder(attr, batch_size);
        }
        else {
            init_tensor(attr, batch_size);
        }
    }
}

void
recordio_protobuf_reader::decoder_state::init_tensor(attribute const &attr,
                                                     std::size_t batch_size)
{
    std::size_t data_size = batch_size * as_size(attr.strides()[0]);

    std::unique_ptr<device_array> arr =
        make_cpu_array(attr.dtype(), data_size);

    size_vector shape = attr.shape();

    // The passed batch size can be less than the actual batch size if
    // there is padding.
    shape[0] = batch_size;

    auto tsr = make_intrusive<dense_tensor>(std::move(shape), std::move(arr));

    tensors.emplace_back(std::move(tsr));

    coo_tensor_builders.emplace_back(nullptr);
}

void
recordio_protobuf_reader::decoder_state::init_coo_tensor_builder(
    attribute const &attr, std::size_t batch_size)
{
    auto bld = make_coo_tensor_builder(attr, batch_size);

    coo_tensor_builders.emplace_back(std::move(bld));

    tensors.emplace_back(nullptr);
}

bool
recordio_protobuf_reader::decoder::decode(std::size_t row_idx,
                                          instance const &ins)
{
    row_idx_ = row_idx;

    instance_ = &ins;

    Record const *proto_msg = parse_proto(ins);
    if (proto_msg == nullptr) {
        return false;
    }

    std::size_t num_features_read = 0;

    for (auto &[label, value] : proto_msg->label()) {
        if (!decode_feature("label_" + label, value)) {
            return false;
        }

        num_features_read++;
    }
    for (auto &[label, value] : proto_msg->features()) {
        if (!decode_feature(label, value)) {
            return false;
        }

        num_features_read++;
    }

    // Make sure that we read all the features for which we
    // have an attribute in the schema.
    if (num_features_read == reader_->get_schema()->attributes().size()) {
        return true;
    }

    if (state_->bbh != bad_batch_handling::skip) {
        auto msg = fmt::format(
            "The record {1:n} in the data store {0} has {2:n} feature(s) "
            "while the expected number of features is {3:n}.",
            instance_->get_data_store(),
            instance_->index() + 1,
            num_features_read,
            reader_->get_schema()->attributes().size());

        if (state_->bbh == bad_batch_handling::error) {
            throw invalid_instance_error{msg};
        }

        logger::warn(msg);
    }

    return false;
}

Record const *
recordio_protobuf_reader::decoder::parse_proto(instance const &ins) const
{
    Record const *proto_msg = recordio_protobuf_reader::parse_proto(ins);
    if (proto_msg != nullptr) {
        return proto_msg;
    }

    if (state_->bbh != bad_batch_handling::skip) {
        auto msg = fmt::format(
            "The record {1:n} in the data store {0} contains a corrupt "
            "RecordIO-protobuf message.",
            instance_->get_data_store(),
            instance_->index() + 1);

        if (state_->bbh == bad_batch_handling::error) {
            throw invalid_instance_error{msg};
        }

        logger::warn(msg);
    }

    return nullptr;
}

bool
recordio_protobuf_reader::decoder::decode_feature(std::string const &name,
                                                  Value const &value)
{
    std::optional<std::size_t> idx = reader_->get_schema()->get_index(name);
    if (idx == std::nullopt) {
        if (state_->bbh != bad_batch_handling::skip) {
            auto msg = fmt::format(
                "The record {1:n} in the data store {0} has an unknown "
                "feature named '{2}'.",
                instance_->get_data_store(),
                instance_->index() + 1,
                name);

            if (state_->bbh == bad_batch_handling::error) {
                throw invalid_instance_error{msg};
            }

            logger::warn(msg);
        }

        return false;
    }

    ftr_idx_ = *idx;

    attr_ = &reader_->get_schema()->attributes()[ftr_idx_];

    switch (value.value_case()) {
    case Value::ValueCase::kFloat32Tensor:
        return decode_feature<data_type::float32>(value.float32_tensor());

    case Value::ValueCase::kFloat64Tensor:
        return decode_feature<data_type::float64>(value.float64_tensor());

    case Value::ValueCase::kInt32Tensor:
        return decode_feature<data_type::sint32>(value.int32_tensor());

    case Value::ValueCase::kBytes:
    case Value::ValueCase::VALUE_NOT_SET:
        break;
    }

    if (state_->bbh != bad_batch_handling::skip) {
        auto msg = fmt::format(
            "The feature '{2}' of the record {1:n} in the data store {0} has "
            "an unexpected data type.",
            instance_->get_data_store(),
            instance_->index() + 1,
            attr_->name());

        if (state_->bbh == bad_batch_handling::error) {
            throw invalid_instance_error{msg};
        }

        logger::warn(msg);
    }

    return false;
}

template<data_type dt, typename ProtobufTensor>
bool
recordio_protobuf_reader::decoder::decode_feature(ProtobufTensor const &tsr)
{
    if (attr_->dtype() != dt) {
        if (state_->bbh != bad_batch_handling::skip) {
            auto msg = fmt::format(
                "The feature '{2}' of the record {1:n} in the data store {0} "
                "has the data type {3}, while the expected data type is {4}.",
                instance_->get_data_store(),
                instance_->index() + 1,
                attr_->name(),
                dt,
                attr_->dtype());

            if (state_->bbh == bad_batch_handling::error) {
                throw invalid_instance_error{msg};
            }

            logger::warn(msg);
        }

        return false;
    }

    if (is_sparse(tsr) != attr_->sparse()) {
        if (state_->bbh != bad_batch_handling::skip) {
            char const *ft{};
            if (attr_->sparse()) {
                ft =
                    "The feature '{2}' of the record {1:n} in the data store "
                    "{0} is sparse, while the expected storage type is dense.";
            }
            else {
                ft =
                    "The feature '{2}' of the record {1:n} in the data store "
                    "{0} is dense, while the expected storage type is sparse.";
            }
            auto msg = fmt::format(ft,
                                   instance_->get_data_store(),
                                   instance_->index() + 1,
                                   attr_->name());

            if (state_->bbh == bad_batch_handling::error) {
                throw invalid_instance_error{msg};
            }

            logger::warn(msg);
        }

        return false;
    }

    if (!shape_equals(tsr)) {
        if (state_->bbh != bad_batch_handling::skip) {
            std::string pshp;
            if (tsr.shape().empty()) {
                pshp = fmt::to_string(tsr.values_size());
            }
            else {
                pshp = fmt::format("{0}", fmt::join(tsr.shape(), ", "));
            }

            size_vector const &shape = attr_->shape();

            auto msg = fmt::format(
                "The feature '{2}' of the record {1:n} in the data store "
                "{0} has the shape ({3}), while the expected shape is ({4}).",
                instance_->get_data_store(),
                instance_->index() + 1,
                attr_->name(),
                pshp,
                fmt::join(shape.begin() + 1, shape.end(), ", "));

            if (state_->bbh == bad_batch_handling::error) {
                throw invalid_instance_error{msg};
            }

            logger::warn(msg);
        }

        return false;
    }

    if (attr_->sparse()) {
        return append_to_builder<dt>(tsr);
    }

    return copy_to_tensor<dt>(tsr);
}

template<typename ProtobufTensor>
bool
recordio_protobuf_reader::decoder::is_sparse(ProtobufTensor const &tsr) const
{
    if (tsr.keys().empty()) {
        return tsr.values().empty() && !tsr.shape().empty();
    }
    return true;
}

template<typename ProtobufTensor>
bool
recordio_protobuf_reader::decoder::shape_equals(
    ProtobufTensor const &tsr) const
{
    size_vector const &shape = attr_->shape();

    // A dense feature might have no shape specified.
    if (tsr.shape().empty()) {
        if (shape.size() == 2) {
            auto val_size = static_cast<std::size_t>(tsr.values_size());
            // In such case we consider the size of the value array as
            // its one-dimensional shape.
            return shape[1] == val_size;
        }
        return false;
    }

    auto shp_size = static_cast<std::size_t>(tsr.shape().size());
    // Note that we should skip the batch dimension while comparing the
    // two shapes.
    if (shape.size() - 1 != shp_size) {
        return false;
    }

    auto pos = shape.begin() + 1;

    for (std::uint64_t dim : tsr.shape()) {
        std::size_t d{};
        if (!try_narrow(dim, d) || *pos != d) {
            return false;
        }

        ++pos;
    }

    return true;
}

template<data_type dt, typename ProtobufTensor>
bool
recordio_protobuf_reader::decoder::copy_to_tensor(
    ProtobufTensor const &tsr) const
{
    // The stride of the batch dimension.
    std::ptrdiff_t num_values = attr_->strides()[0];

    if (num_values != tsr.values_size()) {
        if (state_->bbh != bad_batch_handling::skip) {
            size_vector const &shape = attr_->shape();

            auto msg = fmt::format(
                "The feature '{2}' of the record {1:n} in the data store {0} "
                "has {3:n} values(s) but a shape of ({4:n}).",
                instance_->get_data_store(),
                instance_->index() + 1,
                attr_->name(),
                tsr.values_size(),
                fmt::join(shape.begin() + 1, shape.end(), ", "));

            if (state_->bbh == bad_batch_handling::error) {
                throw invalid_instance_error{msg};
            }

            logger::warn(msg);
        }

        return false;
    }

    auto dest = static_cast<dense_tensor &>(*state_->tensors[ftr_idx_])
                    .data()
                    .as<data_type_t<dt>>();

    std::ptrdiff_t offset = as_ssize(row_idx_) * num_values;

    std::copy_n(tsr.values().begin(), num_values, dest.begin() + offset);

    return true;
}

template<data_type dt, typename ProtobufTensor>
bool
recordio_protobuf_reader::decoder::append_to_builder(
    ProtobufTensor const &tsr) const
{
    if (tsr.keys_size() != tsr.values_size()) {
        if (state_->bbh != bad_batch_handling::skip) {
            auto msg = fmt::format(
                "The sparse feature '{2}' of the record {1:n} in the data "
                "store {0} has {3:n} key(s) but {4:n} value(s).",
                instance_->get_data_store(),
                instance_->index() + 1,
                attr_->name(),
                tsr.keys_size(),
                tsr.values_size());

            if (state_->bbh == bad_batch_handling::error) {
                throw invalid_instance_error{msg};
            }

            logger::warn(msg);
        }

        return false;
    }

    auto &bld = static_cast<coo_tensor_builder_impl<dt> &>(
        *state_->coo_tensor_builders[ftr_idx_]);

    if (bld.append(tsr.values(), tsr.keys())) {
        return true;
    }

    if (state_->bbh != bad_batch_handling::skip) {
        auto msg = fmt::format(
            "The sparse feature '{2}' of the record {1:n} in the data "
            "store {0} has one or more invalid keys.",
            instance_->get_data_store(),
            instance_->index() + 1,
            attr_->name());

        if (state_->bbh == bad_batch_handling::error) {
            throw invalid_instance_error{msg};
        }

        logger::warn(msg);
    }

    return false;
}

}  // namespace v1
}  // namespace mlio
