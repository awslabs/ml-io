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

#include "mlio/image_reader.h"

#ifdef MLIO_BUILD_IMAGE_READER

#include <cstddef>
#include <stdexcept>

#include <fmt/format.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "mlio/cpu_array.h"
#include "mlio/data_reader_error.h"
#include "mlio/data_stores/data_store.h"
#include "mlio/data_type.h"
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/logger.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/record_readers/recordio_record_reader.h"
#include "mlio/schema.h"
#include "mlio/util/cast.h"

namespace mlio {
inline namespace abi_v1 {

Image_reader::Image_reader(Data_reader_params params, Image_reader_params img_params)
    : Parallel_data_reader{std::move(params)}, params_{std::move(img_params)}
{
    if (params_.image_dimensions.size() != image_dimensions_size_) {
        throw std::invalid_argument{
            "The dimensions of the output image must be entered in (channels, height, width) format."};
    }

    std::copy(params_.image_dimensions.begin(), params_.image_dimensions.end(), img_dims_.begin());

    error_bad_example_ = this->params().bad_example_handling == Bad_example_handling::error;
}

Image_reader::~Image_reader()
{
    stop();
}

Intrusive_ptr<Record_reader> Image_reader::make_record_reader(const Data_store &store)
{
    switch (params_.image_frame) {
    case Image_frame::none:
        return nullptr;
    case Image_frame::recordio:
        return make_intrusive<detail::Recordio_record_reader>(store.open_read());
    }

    throw std::invalid_argument{"The specified image frame is invalid."};
}

Intrusive_ptr<const Schema> Image_reader::infer_schema(const std::optional<Instance> &)
{
    std::vector<Attribute> attrs{};
    // The schema follows the NHWC convention.
    attrs.emplace_back("value",
                       Data_type::uint8,
                       Size_vector{params().batch_size,
                                   params_.image_dimensions[1],
                                   params_.image_dimensions[2],
                                   params_.image_dimensions[0]});

    return make_intrusive<Schema>(std::move(attrs));
}

Intrusive_ptr<Example> Image_reader::decode(const Instance_batch &batch) const
{
    // The stride of the batch dimension corresponds to the byte size
    // of the images contained in the example.
    auto batch_stride = as_size(schema()->attributes()[0].strides()[0]);

    auto tensor = make_tensor(batch.size(), batch_stride);

    auto bits = tensor->data().as<std::uint8_t>();

    std::size_t num_instances_read = 0;

    for (const Instance &instance : batch.instances()) {
        if (decode_core(bits, instance)) {
            bits = bits.subspan(batch_stride);

            num_instances_read++;
        }
        else {
            // If the user requested to skip the example in case of an
            // error, shortcut the loop and return immediately.
            if (params().bad_example_handling == Bad_example_handling::skip) {
                return {};
            }
            if (params().bad_example_handling == Bad_example_handling::skip_warn) {
                logger::warn(
                    "The example #{0:n} has been skipped as it had at least one bad instance.",
                    batch.index());

                return {};
            }
            if (params().bad_example_handling != Bad_example_handling::pad &&
                params().bad_example_handling != Bad_example_handling::pad_warn) {
                throw std::invalid_argument{"The specified bad example handling is invalid."};
            }
        }
    }

    if (batch.instances().size() != num_instances_read) {
        if (params().bad_example_handling == Bad_example_handling::pad_warn) {
            logger::warn("The example #{0:n} has been padded as it had {1:n} bad instance(s).",
                         batch.index(),
                         batch.instances().size() - num_instances_read);
        }
    }

    std::vector<Intrusive_ptr<Tensor>> tensors{};
    tensors.emplace_back(std::move(tensor));

    auto example = make_intrusive<Example>(schema(), std::move(tensors));

    example->padding = batch.size() - num_instances_read;

    return example;
}

Intrusive_ptr<Dense_tensor>
Image_reader::make_tensor(std::size_t batch_size, std::size_t batch_stride) const
{
    Size_vector shape{batch_size,
                      params_.image_dimensions[1],
                      params_.image_dimensions[2],
                      params_.image_dimensions[0]};

    auto arr = make_cpu_array(Data_type::uint8, batch_size * batch_stride);

    return make_intrusive<Dense_tensor>(std::move(shape), std::move(arr));
}

bool Image_reader::decode_core(stdx::span<std::uint8_t> out, const Instance &instance) const
{
    Memory_slice img_buf{};
    if (params_.image_frame == Image_frame::recordio) {
        // Skip the 24-byte header defined in image_recordio.h in the
        // MXNet GitHub repository. This header contains metadata and
        // is not relevant in our implementation.
        img_buf = instance.bits().subslice(recordio_image_header_offset_);
    }
    else {
        img_buf = instance.bits();
    }

    cv::Mat mat{1,
                static_cast<int>(img_buf.size()),
                CV_8U,
                // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
                static_cast<void *>(const_cast<std::byte *>(img_buf.data()))};

    cv::ImreadModes mode{};
    int type{};

    switch (img_dims_[0]) {
    case 1:
        mode = cv::ImreadModes::IMREAD_GRAYSCALE;
        type = CV_8UC1;
        break;
    case 3:
        mode = cv::ImreadModes::IMREAD_COLOR;
        type = CV_8UC3;
        break;
    case 4:
        mode = cv::ImreadModes::IMREAD_UNCHANGED;
        type = CV_8UC4;
        break;
    default:
        throw std::invalid_argument{fmt::format(
            "The specified image dimensions have an unsupported number of channels ({0:n}).",
            img_dims_[0])};
    }

    cv::Mat tmp = decode_image(mat, mode, instance);
    if (tmp.empty()) {
        return false;
    }

    if (params_.resize) {
        if (!resize(tmp, tmp, instance)) {
            return false;
        }
    }

    if (params_.to_rgb && tmp.channels() != 1) {
        try {
            cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);
        }
        catch (const cv::Exception &e) {
            if (warn_bad_instances() || error_bad_example_) {
                auto msg = fmt::format(
                    "The BGR2RGB operation failed for the image #{1:n} in the data store '{0}' with the following exception: {2}",
                    instance.data_store().id(),
                    instance.index(),
                    e.what());

                if (warn_bad_instances()) {
                    logger::warn(msg);
                }

                if (error_bad_example_) {
                    throw Invalid_instance_error{msg};
                }
            }

            return false;
        }
    }

    cv::Mat dst{img_dims_[1], img_dims_[2], type, out.data()};

    return crop(tmp, dst, instance);
}

cv::Mat Image_reader::decode_image(const cv::Mat &buf, int mode, const Instance &instance) const
{
    cv::Mat decoded_img{};
    try {
        decoded_img = cv::imdecode(buf, mode);
    }
    catch (const cv::Exception &e) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The image decode operation failed for the image #{1:n} in the data store '{0}' with the following exception: {2}",
                instance.data_store().id(),
                instance.index(),
                e.what());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw Invalid_instance_error{msg};
            }
        }

        return decoded_img;
    }

    if (decoded_img.empty()) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The image decode operation failed for the image #{1:n} in the data store '{0}'.",
                instance.data_store().id(),
                instance.index());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw Invalid_instance_error{msg};
            }
        }

        return decoded_img;
    }

    if (mode == cv::ImreadModes::IMREAD_UNCHANGED && decoded_img.channels() != 4) {
        throw std::invalid_argument{fmt::format(
            "The image #{1:n} in the data store '{0}' contains {2:n} channels while it is expected to contain 4 channels.",
            instance.data_store().id(),
            instance.index(),
            decoded_img.channels())};
    }

    return decoded_img;
}

bool Image_reader::resize(cv::Mat &src, cv::Mat &dst, const Instance &instance) const
{
    int new_cols{};
    int new_rows{};

    int resize_value = static_cast<int>(*params_.resize);

    if (src.rows > src.cols) {
        new_cols = resize_value * src.rows / src.cols;
        new_rows = resize_value;
    }
    else {
        new_cols = resize_value;
        new_rows = resize_value * src.cols / src.rows;
    }

    try {
        cv::resize(src, dst, cv::Size{new_rows, new_cols}, 0, 0);
    }
    catch (const cv::Exception &e) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The image resize operation failed for the image #{2:n} in the data store '{0}' with the following exception: {2}",
                instance.data_store().id(),
                instance.index(),
                e.what());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    return true;
}

bool Image_reader::crop(cv::Mat &src, cv::Mat &dst, const Instance &instance) const
{
    if (src.rows < img_dims_[1] || src.cols < img_dims_[2]) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The input image dimensions (rows: {2:n}, cols: {3:n}) are smaller than the output image dimensions (rows: {4:n}, cols: {5:n}) for the image #{1:n} in the data store '{0}'.",
                instance.data_store().id(),
                instance.index(),
                src.rows,
                src.cols,
                img_dims_[1],
                img_dims_[2]);

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw std::invalid_argument{msg};
            }
        }

        return false;
    }

    // Crop from the center.
    int y = (src.rows - img_dims_[1]) / 2;
    int x = (src.cols - img_dims_[2]) / 2;

    try {
        cv::Rect roi{x, y, img_dims_[2], img_dims_[1]};
        src(roi).copyTo(dst);
    }
    catch (const cv::Exception &e) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The image crop operation failed for the image #{1:n} in the data store '{0}' with the following exception: {2}",
                instance.data_store().id(),
                instance.index(),
                e.what());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw Invalid_instance_error{msg};
            }
        }

        return false;
    }

    return true;
}

}  // namespace abi_v1
}  // namespace mlio

#else

#include "mlio/Not_supported_error.h"
#include "mlio/record_readers/Record_reader.h"

namespace cv {

class Mat {};

}  // namespace cv

namespace mlio {
inline namespace abi_v1 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"

// NOLINTNEXTLINE(performance-unnecessary-value-param)
Image_reader::Image_reader(Data_reader_params params, Image_reader_params)
    : Parallel_data_reader{std::move(params)}, params_{}, error_bad_example_{}
{
    throw Not_supported_error{"MLIO was not built with image reader support."};
}

Image_reader::~Image_reader() = default;

Intrusive_ptr<Record_reader> Image_reader::make_record_reader(const Data_store &)
{
    return nullptr;
}

Intrusive_ptr<const Schema> Image_reader::infer_schema(const std::optional<Instance> &)
{
    return nullptr;
}

Intrusive_ptr<Example> Image_reader::decode(const Instance_batch &) const
{
    return nullptr;
}

#pragma GCC diagnostic pop

}  // namespace abi_v1
}  // namespace mlio

#endif
