/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <fmt/format.h>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "mlio/cpu_array.h"
#include "mlio/data_type.h"
#include "mlio/instance.h"
#include "mlio/instance_batch.h"
#include "mlio/intrusive_ptr.h"
#include "mlio/logger.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/record_readers/blob_record_reader.h"
#include "mlio/record_readers/recordio_record_reader.h"
#include "mlio/schema.h"

namespace mlio {
inline namespace v1 {

image_reader::image_reader(data_reader_params rdr_prm,
                           image_reader_params image_prm)
    : parallel_data_reader{std::move(rdr_prm)}, params_{std::move(image_prm)}
{
    if (params_.image_dimensions.size() != image_dimensions_size_) {
        throw std::invalid_argument{
            "image_dimensions is a required parameter. "
            "Dimensions of the output image must be entered in "
            "(channels, height, width) format"};
    }
    bbh_ = effective_bad_batch_handling();
    std::copy(params_.image_dimensions.begin(),
              params_.image_dimensions.end(),
              img_dims_.begin());
}

image_reader::~image_reader()
{
    stop();
}

intrusive_ptr<record_reader>
image_reader::make_record_reader(data_store const &ds)
{
    switch (params_.img_frame) {
    case image_frame::none:
        return make_intrusive<blob_record_reader>(ds.open_read());
    case image_frame::recordio:
        return make_intrusive<detail::recordio_record_reader>(ds.open_read());
    }
    throw std::invalid_argument{
        "The image_frame argument does not represent a valid value."};
}

intrusive_ptr<schema const>
image_reader::infer_schema(std::optional<instance> const &)
{
    std::vector<attribute> attrs{};
    // schema follows the NHWC convention
    attrs.emplace_back(attribute_builder{"value",
                                            data_type::uint8,
                                            {params().batch_size,
                                             params_.image_dimensions[1],
                                             params_.image_dimensions[2],
                                             params_.image_dimensions[0]}}
                           .build());
    return make_intrusive<schema>(std::move(attrs));
}

intrusive_ptr<example>
image_reader::decode(instance_batch const &batch) const
{
    auto tsr = make_tensor(batch.instances(), batch.size());
    if (tsr == nullptr) {
        return nullptr;
    }
    std::vector<intrusive_ptr<tensor>> tensors{};
    tensors.emplace_back(std::move(tsr));
    return make_intrusive<example>(get_schema(), std::move(tensors));
}

intrusive_ptr<dense_tensor>
image_reader::make_tensor(std::vector<instance> const &instances,
                          std::size_t batch_size) const
{
    auto instance_offset = params_.image_dimensions[0] *
                           params_.image_dimensions[1] *
                           params_.image_dimensions[2];
    auto total_memory_buffer = instance_offset * batch_size;
    std::vector<std::uint8_t> img_vec(total_memory_buffer);
    auto *ptr_img_vec = img_vec.data();
    int type{};
    cv::ImreadModes mode{};
    for (instance const &ins : instances) {
        memory_slice img_buf;
        if (params_.img_frame == image_frame::recordio) {
            // This is to skip the 24 byte header as mentioned in
            // image_recordio.h in the mxnet github repository. This header
            // does not contain data related to image pixel values, but rather
            // contains image metadata.
            img_buf = ins.bits().subslice(recordio_image_header_offset_);
        }
        else {
            img_buf = ins.bits();
        }
        cv::Mat mat(
            1,
            static_cast<int>(img_buf.size()),
            CV_8U,
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
            static_cast<void *>(const_cast<std::byte *>(img_buf.data())));
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
            throw std::invalid_argument{
                fmt::format("Unsupported number of channels entered in the "
                            "image_dimensions parameter: {0:n}",
                            params_.image_dimensions[0])};
        }
        cv::Mat tmp = decode_image(mat, mode, ins);
        if (tmp.empty()) {
            return nullptr;
        }
        if (params_.resize.has_value()) {
            if (!resize(tmp, tmp, ins)) {
                return nullptr;
            }
        }
        if (params_.to_rgb && tmp.channels() != 1) {
            try {
                cv::cvtColor(tmp, tmp, cv::COLOR_BGR2RGB);
            }
            catch (cv::Exception const &e) {
                if (bbh_ != bad_batch_handling::skip) {
                    auto msg =
                        fmt::format("BGR2RGB operation for image: {0:n} "
                                    "in data_store: {1:s} failed with the "
                                    "following exception: {2:s}.",
                                    ins.index(),
                                    ins.get_data_store().id(),
                                    e.what());
                    if (bbh_ == bad_batch_handling::error) {
                        throw std::runtime_error{msg};
                    }
                    logger::warn(msg);
                }
                // skip batch
                return nullptr;
            }
        }
        cv::Mat dst{img_dims_[1], img_dims_[2], type, ptr_img_vec};
        if (!crop(tmp, dst, ins)) {
            return nullptr;
        }
        ptr_img_vec += instance_offset;
    }

    auto ptr = wrap_cpu_array<data_type::uint8>(std::move(img_vec));
    auto tsr =
        make_intrusive<dense_tensor>(size_vector{batch_size,
                                                 params_.image_dimensions[1],
                                                 params_.image_dimensions[2],
                                                 params_.image_dimensions[0]},
                                     std::move(ptr));
    return tsr;
}

cv::Mat
image_reader::decode_image(cv::Mat const &buf,
                           int mode,
                           instance const &ins) const
{
    cv::Mat decoded_img;
    try {
        decoded_img = cv::imdecode(buf, mode);
    }
    catch (cv::Exception const &e) {
        if (bbh_ != bad_batch_handling::skip) {
            auto msg = fmt::format(
                "Image decoding failed for the image: {0:n} in data_store: "
                "{1:s}, with the following exception: {2:s}",
                ins.index(),
                ins.get_data_store().id(),
                e.what());
            if (bbh_ == bad_batch_handling::error) {
                throw std::runtime_error{msg};
            }
            logger::warn(msg);
        }
        return decoded_img;
    }
    if (decoded_img.empty()) {
        if (bbh_ != bad_batch_handling::skip) {
            auto msg =
                fmt::format("Image decoding failed for the image: {0:n} in "
                            "data_store: {1:s}.",
                            ins.index(),
                            ins.get_data_store().id());
            if (bbh_ == bad_batch_handling::error) {
                throw std::runtime_error{msg};
            }
            logger::warn(msg);
        }
        return decoded_img;
    }
    if (mode == cv::ImreadModes::IMREAD_UNCHANGED &&
        decoded_img.channels() != 4) {
        throw std::invalid_argument{fmt::format(
            "Image: {0:n} in data_store: {1:s} is expected to have "
            "4 channels. However, it contains {2:n} channels.",
            ins.index(),
            ins.get_data_store().id(),
            decoded_img.channels())};
    }
    return decoded_img;
}

bool
image_reader::resize(cv::Mat &src, cv::Mat &dst, instance const &ins) const
{
    int new_height{};
    int new_width{};
    int resize_value = static_cast<int>(params_.resize.value());

    if (src.rows > src.cols) {
        new_height = (resize_value * src.rows) / src.cols;
        new_width = resize_value;
    }
    else {
        new_height = resize_value;
        new_width = (resize_value * src.cols) / src.rows;
    }
    try {
        cv::resize(src, dst, cv::Size(new_width, new_height), 0, 0);
    }
    catch (cv::Exception const &e) {
        if (bbh_ != bad_batch_handling::skip) {
            auto msg = fmt::format(
                "Image resize operation failed for the image: {0:n} in "
                "data_store: {1:s}, with the following exception: {2:s}",
                ins.index(),
                ins.get_data_store().id(),
                e.what());
            if (bbh_ == bad_batch_handling::error) {
                throw std::runtime_error{msg};
            }
            logger::warn(msg);
        }
        return false;
    }
    return true;
}

bool
image_reader::crop(cv::Mat &src, cv::Mat &dst, instance const &ins) const
{
    if (src.rows < img_dims_[1] || src.cols < img_dims_[2]) {
        if (bbh_ != bad_batch_handling::skip) {
            auto msg = fmt::format(
                "Input image dimensions [rows: {0:n}, cols: {1:n}] are "
                "smaller than the output image image_dimensions [rows: {2:n}, "
                "cols: {3:n}], for the image: {4:n} in data_store: {5:s}",
                src.rows,
                src.cols,
                img_dims_[1],
                img_dims_[2],
                ins.index(),
                ins.get_data_store().id());
            if (bbh_ == bad_batch_handling::error) {
                throw std::invalid_argument{msg};
            }
            logger::warn(msg);
        }
        return false;
    }

    // center crop
    int y = (src.rows - img_dims_[1]) / 2;
    int x = (src.cols - img_dims_[2]) / 2;

    try {
        cv::Rect roi(x, y, img_dims_[2], img_dims_[1]);
        src(roi).copyTo(dst);
    }
    catch (cv::Exception const &e) {
        if (bbh_ != bad_batch_handling::skip) {
            auto msg = fmt::format(
                "Image crop operation failed for the image: {0:n} in "
                "data_store: {1:s}, with the following exception: {2:s}",
                ins.index(),
                ins.get_data_store().id(),
                e.what());
            if (bbh_ == bad_batch_handling::error) {
                throw std::runtime_error{msg};
            }
            logger::warn(msg);
        }
        return false;
    }
    return true;
}

}  // namespace v1
}  // namespace mlio

#else

// clang-format off

#include "mlio/not_supported_error.h"

namespace cv {
class Mat {};
}  // namespace cv

namespace mlio {
inline namespace v1 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"

image_reader::image_reader(data_reader_params rdr_prm,
                           image_reader_params image_prm)
    : parallel_data_reader{std::move(rdr_prm)}, params_{std::move(image_prm)}
{
    bbh_ = {};
    img_dims_ = {};
    throw not_supported_error{"ML-IO has not been built with image reader "
                              "support."};
}

image_reader::~image_reader() = default;

intrusive_ptr<record_reader>
image_reader::make_record_reader(data_store const &)
{
   return nullptr;
}

intrusive_ptr<schema const>
image_reader::infer_schema(std::optional<instance> const &)
{
    return nullptr;
}

intrusive_ptr<example>
image_reader::decode(instance_batch const &) const
{
    return nullptr;
}

intrusive_ptr<dense_tensor>
image_reader::make_tensor(std::vector<instance> const &, std::size_t) const
{
    return nullptr;
}

cv::Mat
image_reader::decode_image(cv::Mat const &, int, instance const &) const
{
    return {};
}

bool
image_reader::resize(cv::Mat &, cv::Mat &, instance const &) const
{
    return false;
}

bool
image_reader::crop(cv::Mat &, cv::Mat &, instance const &) const
{
    return false;
}

#pragma GCC diagnostic pop

}  // namespace v1
}  // namespace mlio

#endif
