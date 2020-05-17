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
inline namespace v1 {

image_reader::image_reader(data_reader_params prm, image_reader_params img_prm)
    : parallel_data_reader{std::move(prm)}, params_{std::move(img_prm)}
{
    if (params_.image_dimensions.size() != image_dimensions_size_) {
        throw std::invalid_argument{
            "The dimensions of the output image must be entered in (channels, height, width) format."};
    }

    std::copy(params_.image_dimensions.begin(), params_.image_dimensions.end(), img_dims_.begin());

    error_bad_example_ = params().bad_example_hnd == bad_example_handling::error;
}

image_reader::~image_reader()
{
    stop();
}

intrusive_ptr<record_reader> image_reader::make_record_reader(const data_store &ds)
{
    switch (params_.img_frame) {
    case image_frame::none:
        return nullptr;
    case image_frame::recordio:
        return make_intrusive<detail::recordio_record_reader>(ds.open_read());
    }

    throw std::invalid_argument{"The specified image frame is invalid."};
}

intrusive_ptr<schema const> image_reader::infer_schema(std::optional<instance> const &)
{
    std::vector<attribute> attrs{};
    // The schema follows the NHWC convention.
    attrs.emplace_back("value",
                       data_type::uint8,
                       size_vector{params().batch_size,
                                   params_.image_dimensions[1],
                                   params_.image_dimensions[2],
                                   params_.image_dimensions[0]});

    return make_intrusive<schema>(std::move(attrs));
}

intrusive_ptr<example> image_reader::decode(const instance_batch &batch) const
{
    // The stride of the batch dimension corresponds to the byte size
    // of the images.
    auto batch_stride = as_size(get_schema()->attributes()[0].strides()[0]);

    auto tsr = make_tensor(batch.size(), batch_stride);

    auto bits = tsr->data().as<std::uint8_t>();

    std::size_t num_instances_read = 0;

    for (const instance &ins : batch.instances()) {
        if (decode_core(bits, ins)) {
            bits = bits.subspan(batch_stride);

            num_instances_read++;
        }
        else {
            // If the user requested to skip the batch in case of an
            // error, shortcut the loop and return immediately.
            if (params().bad_example_hnd == bad_example_handling::skip) {
                return {};
            }
            if (params().bad_example_hnd == bad_example_handling::skip_warn) {
                logger::warn(
                    "The example #{0:n} has been skipped as it had at least one bad instance.",
                    batch.index());

                return {};
            }
            if (params().bad_example_hnd != bad_example_handling::pad &&
                params().bad_example_hnd != bad_example_handling::pad_warn) {
                throw std::invalid_argument{"The specified bad batch handling is invalid."};
            }
        }
    }

    if (batch.instances().size() != num_instances_read) {
        if (params().bad_example_hnd == bad_example_handling::pad_warn) {
            logger::warn("The example #{0:n} has been padded as it had {1:n} bad instance(s).",
                         batch.index(),
                         batch.instances().size() - num_instances_read);
        }
    }

    std::vector<intrusive_ptr<tensor>> tensors{};
    tensors.emplace_back(std::move(tsr));

    auto exm = make_intrusive<example>(get_schema(), std::move(tensors));

    exm->padding = batch.size() - num_instances_read;

    return exm;
}

intrusive_ptr<dense_tensor>
image_reader::make_tensor(std::size_t batch_size, std::size_t batch_stride) const
{
    size_vector shp{batch_size,
                    params_.image_dimensions[1],
                    params_.image_dimensions[2],
                    params_.image_dimensions[0]};

    auto arr = make_cpu_array(data_type::uint8, batch_size * batch_stride);

    return make_intrusive<dense_tensor>(std::move(shp), std::move(arr));
}

bool image_reader::decode_core(stdx::span<std::uint8_t> out, const instance &ins) const
{
    memory_slice img_buf{};
    if (params_.img_frame == image_frame::recordio) {
        // Skip the 24-byte header defined in image_recordio.h in the
        // MXNet GitHub repository. This header contains metadata and
        // is not relevant.
        img_buf = ins.bits().subslice(recordio_image_header_offset_);
    }
    else {
        img_buf = ins.bits();
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

    cv::Mat tmp = decode_image(mat, mode, ins);
    if (tmp.empty()) {
        return false;
    }

    if (params_.resize) {
        if (!resize(tmp, tmp, ins)) {
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
                    ins.get_data_store().id(),
                    ins.index(),
                    e.what());

                if (warn_bad_instances()) {
                    logger::warn(msg);
                }

                if (error_bad_example_) {
                    throw std::runtime_error{msg};
                }
            }

            return false;
        }
    }

    cv::Mat dst{img_dims_[1], img_dims_[2], type, out.data()};

    return crop(tmp, dst, ins);
}

cv::Mat image_reader::decode_image(const cv::Mat &buf, int mode, const instance &ins) const
{
    cv::Mat decoded_img{};
    try {
        decoded_img = cv::imdecode(buf, mode);
    }
    catch (const cv::Exception &e) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The image decode operation failed for the image #{1:n} in the data store '{0}' with the following exception: {2}",
                ins.get_data_store().id(),
                ins.index(),
                e.what());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw std::runtime_error{msg};
            }
        }

        return decoded_img;
    }

    if (decoded_img.empty()) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The image decode operation failed for the image #{1:n} in the data store '{0}'.",
                ins.get_data_store().id(),
                ins.index());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw std::runtime_error{msg};
            }
        }

        return decoded_img;
    }

    if (mode == cv::ImreadModes::IMREAD_UNCHANGED && decoded_img.channels() != 4) {
        throw std::invalid_argument{fmt::format(
            "The image #{1:n} in the data store '{0}' is expected to have 4 channels. However it contains {2:n} channels.",
            ins.get_data_store().id(),
            ins.index(),
            decoded_img.channels())};
    }

    return decoded_img;
}

bool image_reader::resize(cv::Mat &src, cv::Mat &dst, const instance &ins) const
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
                ins.get_data_store().id(),
                ins.index(),
                e.what());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw std::runtime_error{msg};
            }
        }

        return false;
    }

    return true;
}

bool image_reader::crop(cv::Mat &src, cv::Mat &dst, const instance &ins) const
{
    if (src.rows < img_dims_[1] || src.cols < img_dims_[2]) {
        if (warn_bad_instances() || error_bad_example_) {
            auto msg = fmt::format(
                "The input image dimensions (rows: {2:n}, cols: {3:n}) are smaller than the output image dimensions (rows: {4:n}, cols: {5:n}) for the image #{1:n} in the data store '{0}'.",
                ins.get_data_store().id(),
                ins.index(),
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
                ins.get_data_store().id(),
                ins.index(),
                e.what());

            if (warn_bad_instances()) {
                logger::warn(msg);
            }

            if (error_bad_example_) {
                throw std::runtime_error{msg};
            }
        }

        return false;
    }

    return true;
}

}  // namespace v1
}  // namespace mlio

#else

#include "mlio/not_supported_error.h"
#include "mlio/record_readers/record_reader.h"

namespace cv {

class Mat {};

}  // namespace cv

namespace mlio {
inline namespace v1 {

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-noreturn"

// NOLINTNEXTLINE(performance-unnecessary-value-param)
image_reader::image_reader(data_reader_params prm, image_reader_params)
    : parallel_data_reader{std::move(prm)}, params_{}, error_bad_example_{}
{
    throw not_supported_error{"MLIO was not built with image reader support."};
}

image_reader::~image_reader() = default;

intrusive_ptr<record_reader> image_reader::make_record_reader(const data_store &)
{
    return nullptr;
}

intrusive_ptr<schema const> image_reader::infer_schema(std::optional<instance> const &)
{
    return nullptr;
}

intrusive_ptr<example> image_reader::decode(const instance_batch &) const
{
    return nullptr;
}

#pragma GCC diagnostic pop

}  // namespace v1
}  // namespace mlio

#endif
