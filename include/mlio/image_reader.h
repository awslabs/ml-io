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

#include <array>
#include <cstdint>

#include "mlio/config.h"
#include "mlio/parallel_data_reader.h"
#include "mlio/span.h"

namespace cv {

class Mat;

}  // namespace cv

namespace mlio {
inline namespace abi_v1 {

/// @addtogroup data_readers Data Readers
/// @{

/// Specifies the frame of an image.
enum class Image_frame {
    none,     ///< The image is contained in a regular File.
    recordio  ///< The image is contained in an MXNet RecordIO Record.
};

struct MLIO_API Image_reader_params final {
    /// See @ref Image_frame.
    Image_frame image_frame{Image_frame::none};
    /// Scales the shorter edge of the image to the specified size
    /// before applying other augmentations.
    std::optional<std::size_t> resize{};
    /// The dimensions of output image in (channels, height, width)
    /// format.
    std::vector<std::size_t> image_dimensions{};
    /// A boolean value indicating whether to convert from BGR to RGB.
    bool to_rgb = false;
};

/// Represents a @ref Data_reader for reading image datasets.
class MLIO_API Image_reader final : public Parallel_data_reader {
public:
    explicit Image_reader(Data_reader_params params, Image_reader_params img_params);

    Image_reader(const Image_reader &) = delete;

    Image_reader &operator=(const Image_reader &) = delete;

    Image_reader(Image_reader &&) = delete;

    Image_reader &operator=(Image_reader &&) = delete;

    ~Image_reader() final;

private:
    MLIO_HIDDEN
    Intrusive_ptr<Record_reader> make_record_reader(const Data_store &store) final;

    MLIO_HIDDEN
    Intrusive_ptr<const Schema> infer_schema(const std::optional<Instance> &instance) final;

    MLIO_HIDDEN
    Intrusive_ptr<Example> decode(const Instance_batch &batch) const final;

    MLIO_HIDDEN
    Intrusive_ptr<Dense_tensor> make_tensor(std::size_t batch_size, std::size_t batch_stride) const;

    MLIO_HIDDEN
    bool decode_core(stdx::span<std::uint8_t> out, const Instance &instance) const;

    MLIO_HIDDEN
    cv::Mat decode_image(const cv::Mat &buf, int mode, const Instance &instance) const;

    MLIO_HIDDEN
    bool resize(cv::Mat &src, cv::Mat &dst, const Instance &instance) const;

    MLIO_HIDDEN
    bool crop(cv::Mat &src, cv::Mat &dst, const Instance &instance) const;

    static constexpr std::size_t image_dimensions_size_ = 3;
    static constexpr std::size_t recordio_image_header_offset_ = 24;

    Image_reader_params params_;
    std::array<int, image_dimensions_size_> img_dims_{};
    bool error_bad_example_;
};

/// @}

}  // namespace abi_v1
}  // namespace mlio
