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

#pragma once

#include <array>
#include <cstdint>

#include "mlio/config.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/span.h"

namespace cv {

class Mat;

}  // namespace cv

namespace mlio {
inline namespace v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Specifies the frame of an image.
enum class image_frame {
    none,     ///< The image is contained in a regular file.
    recordio  ///< The image is contained in an MXNet RecordIO record.
};

struct MLIO_API image_reader_params final {
    /// See @ref image_frame.
    image_frame img_frame{image_frame::none};
    /// Scales the shorter edge of the image to the specified size
    /// before applying other augmentations.
    std::optional<std::size_t> resize{};
    /// The dimensions of output image in (channels, height, width)
    /// format.
    std::vector<std::size_t> image_dimensions{};
    /// A boolean value indicating whether to convert from BGR to RGB.
    bool to_rgb = false;
};

/// Represents a @ref data_reader for reading image datasets.
class MLIO_API image_reader final : public parallel_data_reader {
private:
    static std::size_t constexpr image_dimensions_size_ = 3;
    static std::size_t constexpr recordio_image_header_offset_ = 24;

public:
    explicit image_reader(data_reader_params prm, image_reader_params img_prm);

    image_reader(image_reader const &) = delete;

    image_reader(image_reader &&) = delete;

    ~image_reader() final;

public:
    image_reader &
    operator=(image_reader const &) = delete;

    image_reader &
    operator=(image_reader &&) = delete;

private:
    MLIO_HIDDEN
    intrusive_ptr<record_reader>
    make_record_reader(data_store const &ds) final;

    MLIO_HIDDEN
    intrusive_ptr<schema const>
    infer_schema(std::optional<instance> const &ins) final;

    MLIO_HIDDEN
    intrusive_ptr<example>
    decode(instance_batch const &batch) const final;

    MLIO_HIDDEN
    intrusive_ptr<dense_tensor>
    make_tensor(std::size_t batch_size, std::size_t batch_stride) const;

    MLIO_HIDDEN
    bool
    decode_core(stdx::span<std::uint8_t> out, instance const &ins) const;

    MLIO_HIDDEN
    cv::Mat
    decode_image(cv::Mat const &buf, int mode, instance const &ins) const;

    MLIO_HIDDEN
    bool
    resize(cv::Mat &src, cv::Mat &dst, instance const &ins) const;

    MLIO_HIDDEN
    bool
    crop(cv::Mat &src, cv::Mat &dst, instance const &ins) const;

private:
    image_reader_params params_;
    std::array<int, image_dimensions_size_> img_dims_{};
    bool error_bad_batch_;
};

/// @}

}  // namespace v1
}  // namespace mlio
