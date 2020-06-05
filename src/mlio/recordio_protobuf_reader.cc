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

#include "mlio/recordio_protobuf_reader.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

#include <fmt/format.h>
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

using mlio::detail::Coo_tensor_builder;
using mlio::detail::Coo_tensor_builder_impl;
using mlio::detail::make_coo_tensor_builder;

namespace mlio {
inline namespace abi_v1 {
namespace detail {
namespace {

// Constructing protobuf message objects is an expensive operation;
// therefore we re-use a single instance per thread.
thread_local aialgs::data::Record proto_msg_{};  // NOLINT(cert-err58-cpp)

}  // namespace
}  // namespace detail

class Recordio_protobuf_reader::Decoder_state {
public:
    explicit Decoder_state(const Recordio_protobuf_reader &r, std::size_t batch_size);

    const Recordio_protobuf_reader *reader;
    bool warn_bad_instance;
    bool error_bad_example;
    std::vector<Intrusive_ptr<Tensor>> tensors{};
    std::vector<std::unique_ptr<Coo_tensor_builder>> coo_tensor_builders{};

private:
    void init_state(const Schema &schema, std::size_t batch_size);

    void init_tensor(const Attribute &attr, std::size_t batch_size);

    void init_coo_tensor_builder(const Attribute &attr, std::size_t batch_size);
};

class Recordio_protobuf_reader::Decoder {
public:
    explicit Decoder(Decoder_state &state) : state_{&state}
    {}

    bool decode(std::size_t row_idx, const Instance &instance);

private:
    const aialgs::data::Record *parse_proto(const Instance &instance) const;

    bool decode_feature(const std::string &name, const aialgs::data::Value &value);

    template<Data_type dt, typename Protobuf_tensor>
    bool decode_feature(const Protobuf_tensor &tensor);

    template<typename Protobuf_tensor>
    bool is_sparse(const Protobuf_tensor &tensor) const;

    template<typename Protobuf_tensor>
    bool shape_equals(const Protobuf_tensor &tensor) const;

    template<Data_type dt, typename Protobuf_tensor>
    bool copy_to_tensor(const Protobuf_tensor &tensor) const;

    template<Data_type dt, typename Protobuf_tensor>
    bool append_to_builder(const Protobuf_tensor &tensor) const;

    Decoder_state *state_;
    const Instance *instance_{};
    std::size_t row_idx_{};
    std::size_t attr_idx_{};
    const Attribute *attr_{};
};

Recordio_protobuf_reader::Recordio_protobuf_reader(Data_reader_params params)
    : Parallel_data_reader{std::move(params)}
{}

Recordio_protobuf_reader::~Recordio_protobuf_reader()
{
    // Make sure that we stop parallel reading before the member objects
    // get destructed; otherwise a background task might try to access
    // an already destructed object.
    stop();
}

Intrusive_ptr<Record_reader> Recordio_protobuf_reader::make_record_reader(const Data_store &store)
{
    return make_intrusive<detail::Recordio_record_reader>(store.open_read());
}

Intrusive_ptr<const Schema>
Recordio_protobuf_reader::infer_schema(const std::optional<Instance> &instance)
{
    if (instance == std::nullopt) {
        return {};
    }

    const aialgs::data::Record *proto_msg = parse_proto(*instance);
    if (proto_msg == nullptr) {
        throw Schema_error{fmt::format(
            "The instance #{1:n} in the data store '{0}' contains a corrupt RecordIO-protobuf message.",
            instance->data_store().id(),
            instance->index())};
    }

    std::vector<Attribute> attrs{};

    for (auto &[label, value] : proto_msg->label()) {
        // The label and feature maps of a RecordIO-protobuf message can
        // contain same-named features. In order to avoid name clashes
        // we use the "label_" prefix for the labels.
        attrs.emplace_back(make_attribute(*instance, "label_" + label, value));
    }
    for (auto &[label, value] : proto_msg->features()) {
        attrs.emplace_back(make_attribute(*instance, label, value));
    }

    auto schema = make_intrusive<Schema>(std::move(attrs));

    // We use this value in the decode function to decide whether the
    // amount of data we need to process is worth to parallelize.
    for (const Attribute &attr : schema->attributes()) {
        if (!attr.sparse()) {
            // Add the stride of the batch dimension.
            num_values_per_instance_ += as_size(attr.strides()[0]);
        }
    }

    return std::move(schema);
}

Attribute Recordio_protobuf_reader::make_attribute(const Instance &instance,
                                                   const std::string &name,
                                                   const aialgs::data::Value &value)
{
    switch (value.value_case()) {
    case aialgs::data::Value::ValueCase::kFloat32Tensor:
        return make_attribute<Data_type::float32>(instance, name, value.float32_tensor());

    case aialgs::data::Value::ValueCase::kFloat64Tensor:
        return make_attribute<Data_type::float64>(instance, name, value.float64_tensor());

    case aialgs::data::Value::ValueCase::kInt32Tensor:
        return make_attribute<Data_type::int32>(instance, name, value.int32_tensor());

    case aialgs::data::Value::ValueCase::kBytes:
        throw Not_supported_error{"The RecordIO-protobuf binary data format is not supported."};

    case aialgs::data::Value::ValueCase::VALUE_NOT_SET:
        break;
    }

    throw Schema_error{fmt::format(
        "The feature '{2}' of the instance #{1:n} in the data store '{0}' has an unsupported data type.",
        instance.data_store().id(),
        instance.index(),
        name)};
}

template<Data_type dt, typename Protobuf_tensor>
Attribute Recordio_protobuf_reader::make_attribute(const Instance &instance,
                                                   const std::string &name,
                                                   const Protobuf_tensor &tensor)
{
    bool sparse{};

    Size_vector shape{params().batch_size};

    if (tensor.keys().empty()) {
        if (tensor.shape().empty()) {
            // If both the shape and the key array are empty, we treat
            // the feature as a dense vector.
            shape.emplace_back(static_cast<std::size_t>(tensor.values_size()));
        }
        else {
            copy_shape(instance, name, shape, tensor);

            // If the feature has neither a key nor a value, there is no
            // way to determine if it is dense or sparse. The common
            // practice is to check whether it has a shape and treat it
            // as sparse in such case.
            if (tensor.values().empty()) {
                sparse = true;
            }
        }
    }
    else {
        sparse = true;

        if (tensor.shape().empty()) {
            throw Schema_error{fmt::format(
                "The sparse feature '{2}' of the instance #{1:n} in the data store '{0}' has no shape specified.",
                instance.data_store().id(),
                instance.index(),
                name)};
        }

        copy_shape(instance, name, shape, tensor);
    }

    if (sparse) {
        has_sparse_feature_ = true;
    }

    return Attribute{name, dt, std::move(shape), {}, sparse};
}

template<typename Protobuf_tensor>
void Recordio_protobuf_reader::copy_shape(const Instance &instance,
                                          const std::string &name,
                                          Size_vector &shape,
                                          const Protobuf_tensor &tensor)
{
    for (std::uint64_t dim : tensor.shape()) {
        std::size_t d{};
        if (!try_narrow(dim, d)) {
            std::size_t s = std::numeric_limits<std::byte>::digits * sizeof(std::size_t);

            throw Schema_error{fmt::format(
                "The shape of the feature '{2}' of the instance #{1:n} in the data store '{0}' has a dimension that is larger than {3}-bits.",
                instance.data_store().id(),
                instance.index(),
                name,
                s)};
        }

        shape.emplace_back(d);
    }
}

Intrusive_ptr<Example> Recordio_protobuf_reader::decode(const Instance_batch &batch) const
{
    Decoder_state state{*this, batch.size()};

    std::size_t num_instances = batch.instances().size();

    constexpr std::size_t cut_off = 10'000'000;

    bool should_run_serial =
        // If we have any sparse features, we cannot decode the example
        // in parallel as we need to append each instance sequentially
        // to the COO tensor.
        has_sparse_feature_ ||
        // If bad example handling mode is pad, we cannot parallelize
        // decoding as good records must be stacked together without
        // any gap in between.
        params().bad_example_handling == Bad_example_handling::pad ||
        params().bad_example_handling == Bad_example_handling::pad_warn ||
        // If the number of values (e.g. integers, floating-points) we
        // need to decode is below the cut-off threshold, avoid parallel
        // execution; otherwise the threading overhead will potentially
        // slow down the performance.
        num_values_per_instance_ * num_instances < cut_off;

    std::optional<std::size_t> num_instances_read{};
    if (should_run_serial) {
        num_instances_read = decode_serial(state, batch);
    }
    else {
        num_instances_read = decode_parallel(state, batch);
    }

    // Check if we failed to decode the example and return a null pointer
    // if that is the case.
    if (num_instances_read == std::nullopt) {
        if (params().bad_example_handling == Bad_example_handling::skip_warn) {
            logger::warn("The example #{0:n} has been skipped as it had at least one bad instance.",
                         batch.index());
        }

        return nullptr;
    }

    if (num_instances != *num_instances_read) {
        if (params().bad_example_handling == Bad_example_handling::pad_warn) {
            logger::warn("The example #{0:n} has been padded as it had {1:n} bad instance(s).",
                         batch.index(),
                         num_instances - *num_instances_read);
        }
    }

    auto tsr_beg = state.tensors.begin();
    auto tsr_end = state.tensors.end();

    auto bld_beg = state.coo_tensor_builders.begin();
    auto bld_end = state.coo_tensor_builders.end();

    auto ftr_beg = tbb::make_zip_iterator(tsr_beg, bld_beg);
    auto ftr_end = tbb::make_zip_iterator(tsr_end, bld_end);

    for (auto ftr_pos = ftr_beg; ftr_pos < ftr_end; ++ftr_pos) {
        Intrusive_ptr<Tensor> &tensor = std::get<0>(*ftr_pos);

        // If no tensor exists at the specified index, it means the
        // corresponding feature is sparse and we should build its
        // COO tensor.
        if (tensor == nullptr) {
            tensor = std::get<1>(*ftr_pos)->build();
        }
    }

    auto example = make_intrusive<Example>(schema(), std::move(state.tensors));

    example->padding = batch.size() - *num_instances_read;

    return example;
}

std::optional<std::size_t>
Recordio_protobuf_reader::decode_serial(Decoder_state &state, const Instance_batch &batch) const
{
    std::size_t row_idx = 0;

    for (const Instance &instance : batch.instances()) {
        Decoder dc{state};
        if (dc.decode(row_idx, instance)) {
            row_idx++;
        }
        else {
            // If the user requested to skip the example in case of an
            // error, shortcut the loop and return immediately.
            if (params().bad_example_handling == Bad_example_handling::skip ||
                params().bad_example_handling == Bad_example_handling::skip_warn) {
                return {};
            }
            if (params().bad_example_handling != Bad_example_handling::pad &&
                params().bad_example_handling != Bad_example_handling::pad_warn) {
                throw std::invalid_argument{"The specified bad example handling is invalid."};
            }
        }
    }

    return row_idx;
}

std::optional<std::size_t>
Recordio_protobuf_reader::decode_parallel(Decoder_state &state, const Instance_batch &batch) const
{
    std::atomic_bool skip_example{};

    std::size_t num_instances = batch.instances().size();

    auto instance_idx_beg = tbb::counting_iterator<std::size_t>(0);
    auto instance_idx_end = tbb::counting_iterator<std::size_t>(num_instances);

    auto instance_beg = batch.instances().begin();
    auto instance_end = batch.instances().end();

    auto range_beg = tbb::make_zip_iterator(instance_idx_beg, instance_beg);
    auto range_end = tbb::make_zip_iterator(instance_idx_end, instance_end);

    tbb::blocked_range<decltype(range_beg)> range{range_beg, range_end};

    auto worker = [this, &state, &skip_example](auto &sub_range) {
        for (auto instance_zip : sub_range) {
            Decoder decoder{state};
            if (!decoder.decode(std::get<0>(instance_zip), std::get<1>(instance_zip))) {
                // If we failed to decode the instance, we can terminate
                // the task right away and skip this example.
                if (params().bad_example_handling == Bad_example_handling::skip ||
                    params().bad_example_handling == Bad_example_handling::skip_warn) {
                    skip_example = true;

                    return;
                }

                throw std::invalid_argument{"The specified bad example handling is invalid."};
            }
        }
    };

    tbb::parallel_for(range, worker, tbb::auto_partitioner{});

    if (skip_example) {
        return {};
    }

    return num_instances;
}

const aialgs::data::Record *Recordio_protobuf_reader::parse_proto(const Instance &instance)
{
    bool parsed = detail::proto_msg_.ParseFromArray(instance.bits().data(),
                                                    static_cast<int>(instance.bits().size()));

    return parsed ? &detail::proto_msg_ : nullptr;
}

Recordio_protobuf_reader::Decoder_state::Decoder_state(const Recordio_protobuf_reader &r,
                                                       std::size_t batch_size)
    : reader{&r}
    , warn_bad_instance{r.warn_bad_instances()}
    , error_bad_example{r.params().bad_example_handling == Bad_example_handling::error}
{
    init_state(*r.schema(), batch_size);
}

void Recordio_protobuf_reader::Decoder_state::init_state(const Schema &schema,
                                                         std::size_t batch_size)
{
    tensors.reserve(schema.attributes().size());

    coo_tensor_builders.reserve(schema.attributes().size());

    for (const Attribute &attr : schema.attributes()) {
        if (attr.sparse()) {
            init_coo_tensor_builder(attr, batch_size);
        }
        else {
            init_tensor(attr, batch_size);
        }
    }
}

void Recordio_protobuf_reader::Decoder_state::init_tensor(const Attribute &attr,
                                                          std::size_t batch_size)
{
    std::size_t data_size = batch_size * as_size(attr.strides()[0]);

    std::unique_ptr<Device_array> arr = make_cpu_array(attr.data_type(), data_size);

    Size_vector shape = attr.shape();

    // The provided batch size can be less than the actual batch size
    // if, for example, we are processing the last batch.
    shape[0] = batch_size;

    auto tensor = make_intrusive<Dense_tensor>(std::move(shape), std::move(arr));

    tensors.emplace_back(std::move(tensor));

    coo_tensor_builders.emplace_back(nullptr);
}

void Recordio_protobuf_reader::Decoder_state::init_coo_tensor_builder(const Attribute &attr,
                                                                      std::size_t batch_size)
{
    auto builder = make_coo_tensor_builder(attr, batch_size);

    coo_tensor_builders.emplace_back(std::move(builder));

    tensors.emplace_back(nullptr);
}

bool Recordio_protobuf_reader::Decoder::decode(std::size_t row_idx, const Instance &instance)
{
    row_idx_ = row_idx;

    instance_ = &instance;

    const aialgs::data::Record *proto_msg = parse_proto(instance);
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

    const auto &schema = state_->reader->schema();

    // Make sure that we read all features for which we have an
    // attribute in the schema.
    if (num_features_read == schema->attributes().size()) {
        return true;
    }

    if (state_->warn_bad_instance || state_->error_bad_example) {
        auto msg = fmt::format(
            "The instance #{1:n} in the data store '{0}' has {2:n} feature(s) while it is expected to have {3:n} features.",
            instance_->data_store().id(),
            instance_->index(),
            num_features_read,
            schema->attributes().size());

        if (state_->warn_bad_instance) {
            logger::warn(msg);
        }

        if (state_->error_bad_example) {
            throw Invalid_instance_error{msg};
        }
    }

    return false;
}

const aialgs::data::Record *
Recordio_protobuf_reader::Decoder::parse_proto(const Instance &instance) const
{
    const aialgs::data::Record *proto_msg = Recordio_protobuf_reader::parse_proto(instance);
    if (proto_msg != nullptr) {
        return proto_msg;
    }

    if (state_->warn_bad_instance || state_->error_bad_example) {
        auto msg = fmt::format(
            "The instance #{1:n} in the data store '{0}' contains a corrupt RecordIO-protobuf message.",
            instance_->data_store().id(),
            instance_->index());

        if (state_->warn_bad_instance) {
            logger::warn(msg);
        }

        if (state_->error_bad_example) {
            throw Invalid_instance_error{msg};
        }
    }

    return nullptr;
}

bool Recordio_protobuf_reader::Decoder::decode_feature(const std::string &name,
                                                       const aialgs::data::Value &value)
{
    const auto &schema = state_->reader->schema();

    std::optional<std::size_t> attr_idx = schema->get_index(name);
    if (attr_idx == std::nullopt) {
        if (state_->warn_bad_instance || state_->error_bad_example) {
            auto msg = fmt::format(
                "The instance #{1:n} in the data store '{0}' has an unknown feature named '{2}'.",
                instance_->data_store().id(),
                instance_->index(),
                name);

            if (state_->warn_bad_instance) {
                logger::warn(msg);
            }

            if (state_->error_bad_example) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    attr_idx_ = *attr_idx;

    attr_ = &schema->attributes()[attr_idx_];

    switch (value.value_case()) {
    case aialgs::data::Value::ValueCase::kFloat32Tensor:
        return decode_feature<Data_type::float32>(value.float32_tensor());

    case aialgs::data::Value::ValueCase::kFloat64Tensor:
        return decode_feature<Data_type::float64>(value.float64_tensor());

    case aialgs::data::Value::ValueCase::kInt32Tensor:
        return decode_feature<Data_type::int32>(value.int32_tensor());

    case aialgs::data::Value::ValueCase::kBytes:
    case aialgs::data::Value::ValueCase::VALUE_NOT_SET:
        break;
    }

    if (state_->warn_bad_instance || state_->error_bad_example) {
        auto msg = fmt::format(
            "The feature '{2}' of the instance #{1:n} in the data store '{0}' has an unexpected data type.",
            instance_->data_store().id(),
            instance_->index(),
            attr_->name());

        if (state_->warn_bad_instance) {
            logger::warn(msg);
        }

        if (state_->error_bad_example) {
            throw Invalid_instance_error{msg};
        }
    }

    return false;
}

template<Data_type dt, typename Protobuf_tensor>
bool Recordio_protobuf_reader::Decoder::decode_feature(const Protobuf_tensor &tensor)
{
    if (attr_->data_type() != dt) {
        if (state_->warn_bad_instance || state_->error_bad_example) {
            auto msg = fmt::format(
                "The feature '{2}' of the instance #{1:n} in the data store '{0}' has the data type {3} while it is expected to have the data type {4}.",
                instance_->data_store().id(),
                instance_->index(),
                attr_->name(),
                dt,
                attr_->data_type());

            if (state_->warn_bad_instance) {
                logger::warn(msg);
            }

            if (state_->error_bad_example) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    if (is_sparse(tensor) != attr_->sparse()) {
        if (state_->warn_bad_instance || state_->error_bad_example) {
            const char *ft{};
            if (attr_->sparse()) {
                ft =
                    "The feature '{2}' of the instance #{1:n} in the data store '{0}' is sparse while it is expected to be dense.";
            }
            else {
                ft =
                    "The feature '{2}' of the instance #{1:n} in the data store '{0}' is dense while it is expected to be sparse.";
            }
            auto msg =
                fmt::format(ft, instance_->data_store().id(), instance_->index(), attr_->name());

            if (state_->warn_bad_instance) {
                logger::warn(msg);
            }

            if (state_->error_bad_example) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    if (!shape_equals(tensor)) {
        if (state_->warn_bad_instance || state_->error_bad_example) {
            std::string shape_str;
            if (tensor.shape().empty()) {
                shape_str = fmt::to_string(tensor.values_size());
            }
            else {
                shape_str = fmt::format("{0}", fmt::join(tensor.shape(), ", "));
            }

            const Size_vector &shape = attr_->shape();

            auto msg = fmt::format(
                "The feature '{2}' of the instance #{1:n} in the data store '{0}' has the shape ({3}) while it is expected to have the shape ({4}).",
                instance_->data_store().id(),
                instance_->index(),
                attr_->name(),
                shape_str,
                fmt::join(shape.begin() + 1, shape.end(), ", "));

            if (state_->warn_bad_instance) {
                logger::warn(msg);
            }

            if (state_->error_bad_example) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    if (attr_->sparse()) {
        return append_to_builder<dt>(tensor);
    }

    return copy_to_tensor<dt>(tensor);
}

template<typename Protobuf_tensor>
bool Recordio_protobuf_reader::Decoder::is_sparse(const Protobuf_tensor &tensor) const
{
    if (tensor.keys().empty()) {
        return tensor.values().empty() && !tensor.shape().empty();
    }
    return true;
}

template<typename Protobuf_tensor>
bool Recordio_protobuf_reader::Decoder::shape_equals(const Protobuf_tensor &tensor) const
{
    const Size_vector &shape = attr_->shape();

    // A dense feature might have no shape specified.
    if (tensor.shape().empty()) {
        if (shape.size() == 2) {
            auto val_size = static_cast<std::size_t>(tensor.values_size());
            // In such case we consider the size of the value array as
            // its one-dimensional shape.
            return shape[1] == val_size;
        }
        return false;
    }

    auto shape_size = static_cast<std::size_t>(tensor.shape().size());
    // We should skip the batch dimension while comparing the shapes.
    if (shape.size() - 1 != shape_size) {
        return false;
    }

    auto pos = shape.begin() + 1;

    for (std::uint64_t dim : tensor.shape()) {
        std::size_t d{};
        if (!try_narrow(dim, d) || *pos != d) {
            return false;
        }

        ++pos;
    }

    return true;
}

template<Data_type dt, typename Protobuf_tensor>
bool Recordio_protobuf_reader::Decoder::copy_to_tensor(const Protobuf_tensor &tensor) const
{
    // The stride of the batch dimension.
    std::ptrdiff_t num_values = attr_->strides()[0];

    if (num_values != tensor.values_size()) {
        if (state_->warn_bad_instance || state_->error_bad_example) {
            const Size_vector &shape = attr_->shape();

            auto msg = fmt::format(
                "The feature '{2}' of the instance #{1:n} in the data store '{0}' has {3:n} values(s) but a shape of ({4:n}).",
                instance_->data_store().id(),
                instance_->index(),
                attr_->name(),
                tensor.values_size(),
                fmt::join(shape.begin() + 1, shape.end(), ", "));

            if (state_->warn_bad_instance) {
                logger::warn(msg);
            }

            if (state_->error_bad_example) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    auto &tsr = static_cast<Dense_tensor &>(*state_->tensors[attr_idx_]);

    auto destination = tsr.data().as<data_type_t<dt>>();

    std::ptrdiff_t offset = as_ssize(row_idx_) * num_values;

    std::copy_n(tensor.values().begin(), num_values, destination.begin() + offset);

    return true;
}

template<Data_type dt, typename Protobuf_tensor>
bool Recordio_protobuf_reader::Decoder::append_to_builder(const Protobuf_tensor &tensor) const
{
    if (tensor.keys_size() != tensor.values_size()) {
        if (state_->warn_bad_instance || state_->error_bad_example) {
            auto msg = fmt::format(
                "The sparse feature '{2}' of the instance #{1:n} in the data store '{0}' has {3:n} key(s) but {4:n} value(s).",
                instance_->data_store().id(),
                instance_->index(),
                attr_->name(),
                tensor.keys_size(),
                tensor.values_size());

            if (state_->warn_bad_instance) {
                logger::warn(msg);
            }

            if (state_->error_bad_example) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    auto &builder =
        static_cast<Coo_tensor_builder_impl<dt> &>(*state_->coo_tensor_builders[attr_idx_]);

    if (builder.append(tensor.values(), tensor.keys())) {
        return true;
    }

    if (state_->warn_bad_instance || state_->error_bad_example) {
        auto msg = fmt::format(
            "The sparse feature '{2}' of the instance #{1:n} in the data store '{0}' has one or more invalid keys.",
            instance_->data_store().id(),
            instance_->index(),
            attr_->name());

        if (state_->warn_bad_instance) {
            logger::warn(msg);
        }

        if (state_->error_bad_example) {
            throw Invalid_instance_error{msg};
        }
    }

    return false;
}

}  // namespace abi_v1
}  // namespace mlio
