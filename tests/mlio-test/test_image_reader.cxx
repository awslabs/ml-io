#include <gtest/gtest.h>
#include <mlio.h>

#include <cstddef>
#include <string>
#include <utility>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace mlio {

class test_image_reader : public ::testing::Test {
protected:
    test_image_reader() = default;

    ~test_image_reader() override;

protected:
    std::string source_dir_{"../resources/images/"};
    std::vector<intrusive_ptr<data_store>> jpeg_dataset_ =
        mlio::list_files(source_dir_, "*.jpg");
    std::vector<intrusive_ptr<data_store>> png_dataset_ =
        mlio::list_files(source_dir_, "*.png");
    std::vector<intrusive_ptr<data_store>> recordio_dataset_ =
        mlio::list_files(source_dir_, "*.rec");

    int ret_img_1channel_ = 1;       // channel
    int ret_img_3channel_ = 3;       // channel
    int ret_img_height_ = 90;        // row
    int ret_img_width_ = 100;        // col
    int ret_img_width_large_ = 150;  // col

    std::vector<std::size_t> image_dimensions_std_{
        static_cast<std::size_t>(ret_img_3channel_),
        static_cast<std::size_t>(ret_img_height_),
        static_cast<std::size_t>(ret_img_width_)};
    std::vector<std::size_t> image_dimensions_1_ch_{
        static_cast<std::size_t>(ret_img_1channel_),
        static_cast<std::size_t>(ret_img_height_),
        static_cast<std::size_t>(ret_img_width_)};
    std::vector<std::size_t> image_dimensions_large_width_{
        static_cast<std::size_t>(ret_img_3channel_),
        static_cast<std::size_t>(ret_img_height_),
        static_cast<std::size_t>(ret_img_width_large_)};

    std::string img0_path_jpg_ = source_dir_ + "test_image_0.jpg";
    std::string img1_path_jpg_ = source_dir_ + "test_image_1.jpg";
    std::string img2_path_jpg_ = source_dir_ + "test_image_2.jpg";
    std::string img0_path_png_ = source_dir_ + "test_image_0.png";

    std::string unsupported_channel_exception_{
        "Unsupported number of channels entered in the image_dimensions "
        "parameter: 5"};
    std::string missing_image_dimensions_prm_exception_{
        "image_dimensions is a required parameter. Dimensions of "
        "the output image must be entered in (channels, height, width) "
        "format"};
    std::string invalid_image_dimensions_exception_{
        "Input image dimensions [rows: 166, cols: 190] are smaller than the "
        "output image image_dimensions [rows: 500, cols: 500], for the image: "
        "0 in data_store:"};

public:
    cv::Mat
    get_expected_image(const std::string &img_path,
                       std::size_t resize = 0,
                       int mode = cv::IMREAD_COLOR)
    {
        cv::Mat img = cv::imread(img_path, mode);
        if (resize > 0) {
            int new_height{};
            int new_width{};
            int resize_value = static_cast<int>(resize);

            if (img.rows > img.cols) {
                new_height = (resize_value * img.rows) / img.cols;
                new_width = resize_value;
            }
            else {
                new_height = resize_value;
                new_width = (resize_value * img.cols) / img.rows;
            }
            cv::resize(img, img, cv::Size(new_width, new_height), 0, 0);
        }
        // crop
        int y = (img.rows - (ret_img_height_)) / 2;
        int x = (img.cols - (ret_img_width_)) / 2;
        cv::Rect roi(x, y, (ret_img_width_), (ret_img_height_));
        return img(roi);
    }

    void
    assert_schema(const schema &sch, size_t batch_size, int channel) const
    {
        ASSERT_EQ(sch.attributes()[0].shape()[0], batch_size);
        ASSERT_EQ(sch.attributes()[0].shape()[1], ret_img_height_);
        ASSERT_EQ(sch.attributes()[0].shape()[2], ret_img_width_);
        ASSERT_EQ(sch.attributes()[0].shape()[3], channel);
    }

    void
    assert_tensor_shape(dense_tensor *tsr,
                        size_t batch_size,
                        int channel) const
    {
        ASSERT_EQ(tsr->shape()[0], batch_size);
        ASSERT_EQ(tsr->shape()[1], ret_img_height_);
        ASSERT_EQ(tsr->shape()[2], ret_img_width_);
        ASSERT_EQ(tsr->shape()[3], channel);
    }
};

test_image_reader::~test_image_reader() = default;

TEST_F(test_image_reader, test_jpg_3_channel_last_batch_dropped)
{
    size_t batch_size = 2;
    mlio::data_reader_params prm{
        jpeg_dataset_, batch_size, {}, {}, last_batch_handling::drop};
    mlio::image_reader_params img_prm{
        image_frame::none, {}, image_dimensions_std_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);
    for (auto i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        while ((exm = reader->read_example()) != nullptr) {
            auto lbl =
                static_cast<dense_tensor *>(exm->find_feature("value").get());
            auto image_buffer = lbl->data().as<std::uint8_t>();
            cv::Mat img_0{
                ret_img_height_, ret_img_width_, CV_8UC3, image_buffer.data()};
            cv::Mat img_1{
                ret_img_height_,
                ret_img_width_,
                CV_8UC3,
                image_buffer.data() +
                    (ret_img_height_ * ret_img_width_ * ret_img_3channel_)};

            ASSERT_NE(image_buffer.data(), nullptr);
            cv::Mat output;
            std::size_t resize = 0;
            cv::bitwise_xor(
                get_expected_image(img0_path_jpg_, resize), img_0, output);
            ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
            cv::bitwise_xor(
                get_expected_image(img1_path_jpg_, resize), img_1, output);
            ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
            auto schema = exm->get_schema();
            assert_schema(schema, batch_size, ret_img_3channel_);
            assert_tensor_shape(lbl, batch_size, ret_img_3channel_);
        }
        reader->reset();
    }
    ASSERT_TRUE(true);
}

TEST_F(test_image_reader, test_jpg_3_channel_with_resize_last_batch_dropped)
{
    size_t batch_size = 2;
    mlio::data_reader_params prm{
        jpeg_dataset_, batch_size, {}, {}, last_batch_handling::drop};
    std::size_t resize = 300;
    mlio::image_reader_params img_prm{
        image_frame::none, resize, image_dimensions_std_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);
    for (auto i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        while ((exm = reader->read_example()) != nullptr) {
            auto lbl =
                static_cast<dense_tensor *>(exm->find_feature("value").get());

            auto image_buffer = lbl->data().as<std::uint8_t>();
            cv::Mat img_0{
                ret_img_height_, ret_img_width_, CV_8UC3, image_buffer.data()};
            cv::Mat img_1{
                ret_img_height_,
                ret_img_width_,
                CV_8UC3,
                image_buffer.data() +
                    (ret_img_height_ * ret_img_width_ * ret_img_3channel_)};

            ASSERT_NE(image_buffer.data(), nullptr);
            cv::Mat output;
            cv::bitwise_xor(
                get_expected_image(img0_path_jpg_, resize), img_0, output);
            ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
            cv::bitwise_xor(
                get_expected_image(img1_path_jpg_, resize), img_1, output);
            ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
            auto schema = exm->get_schema();
            assert_schema(schema, batch_size, ret_img_3channel_);
            assert_tensor_shape(lbl, batch_size, ret_img_3channel_);
        }
        reader->reset();
    }
}

TEST_F(test_image_reader, test_jpg_1_channel)
{
    size_t batch_size = 2;
    mlio::data_reader_params prm{jpeg_dataset_, batch_size};
    std::size_t resize = 300;
    mlio::image_reader_params img_prm{
        image_frame::none, resize, image_dimensions_1_ch_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);

    auto exm = reader->read_example();
    auto lbl = static_cast<dense_tensor *>(exm->find_feature("value").get());
    auto image_buffer = lbl->data().as<std::uint8_t>();
    cv::Mat img_0{
        ret_img_height_, ret_img_width_, CV_8UC1, image_buffer.data()};
    cv::Mat img_1{ret_img_height_,
                  ret_img_width_,
                  CV_8UC1,
                  image_buffer.data() +
                      (ret_img_height_ * ret_img_width_ * ret_img_1channel_)};

    ASSERT_NE(image_buffer.data(), nullptr);
    cv::Mat output;
    cv::bitwise_xor(
        get_expected_image(img0_path_jpg_, resize, cv::IMREAD_GRAYSCALE),
        img_0,
        output);
    ASSERT_EQ(cv::countNonZero(output), 0);
    cv::bitwise_xor(
        get_expected_image(img1_path_jpg_, resize, cv::IMREAD_GRAYSCALE),
        img_1,
        output);
    auto schema = exm->get_schema();
    assert_schema(schema, batch_size, ret_img_1channel_);
    assert_tensor_shape(lbl, batch_size, ret_img_1channel_);
}

TEST_F(test_image_reader, test_png_3_channel_happy_path)
{
    size_t batch_size = 1;
    mlio::data_reader_params prm{png_dataset_, batch_size};
    mlio::image_reader_params img_prm{
        image_frame::none, {}, image_dimensions_std_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);

    auto exm = reader->read_example();
    auto lbl = static_cast<dense_tensor *>(exm->find_feature("value").get());
    auto image_buffer = lbl->data().as<std::uint8_t>();
    cv::Mat img_0{
        ret_img_height_, ret_img_width_, CV_8UC3, image_buffer.data()};

    ASSERT_NE(image_buffer.data(), nullptr);
    cv::Mat output;
    std::size_t resize = 0;
    cv::bitwise_xor(get_expected_image(img0_path_png_, resize), img_0, output);
    ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
    auto schema = exm->get_schema();
    assert_schema(schema, batch_size, ret_img_3channel_);
    assert_tensor_shape(lbl, batch_size, ret_img_3channel_);
}

TEST_F(test_image_reader, test_png_3_channel_happy_path_with_resize)
{
    size_t batch_size = 1;
    mlio::data_reader_params prm{png_dataset_, 1};
    std::size_t resize = 100;
    mlio::image_reader_params img_prm{
        image_frame::none, resize, image_dimensions_std_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);

    auto exm = reader->read_example();
    auto lbl = static_cast<dense_tensor *>(exm->find_feature("value").get());
    auto image_buffer = lbl->data().as<std::uint8_t>();
    cv::Mat img{ret_img_height_, ret_img_width_, CV_8UC3, image_buffer.data()};

    ASSERT_NE(image_buffer.data(), nullptr);
    cv::Mat output;
    cv::bitwise_xor(get_expected_image(img0_path_png_, resize), img, output);
    ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
    auto schema = exm->get_schema();
    assert_schema(schema, batch_size, ret_img_3channel_);
    assert_tensor_shape(lbl, batch_size, ret_img_3channel_);
}

TEST_F(test_image_reader, test_png_1_channel)
{
    size_t batch_size = 1;
    mlio::data_reader_params prm{png_dataset_, 1};
    std::size_t resize = 100;
    mlio::image_reader_params img_prm{
        image_frame::none, resize, image_dimensions_1_ch_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);

    auto exm = reader->read_example();
    auto lbl = static_cast<dense_tensor *>(exm->find_feature("value").get());
    auto image_buffer = lbl->data().as<std::uint8_t>();
    cv::Mat img{ret_img_height_, ret_img_width_, CV_8UC1, image_buffer.data()};

    ASSERT_NE(image_buffer.data(), nullptr);
    cv::Mat output;
    cv::bitwise_xor(
        get_expected_image(img0_path_png_, resize, cv::IMREAD_GRAYSCALE),
        img,
        output);
    ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
    auto schema = exm->get_schema();
    assert_schema(schema, batch_size, ret_img_1channel_);
    assert_tensor_shape(lbl, batch_size, ret_img_1channel_);
}

TEST_F(test_image_reader, test_invalid_image_dimensions_parameter)
{
    // invalid image_dimensions. crop_width and crop_height are larger than the
    // input image
    mlio::data_reader_params prm{jpeg_dataset_, 2};
    std::size_t crop_width = 500;
    std::size_t crop_height = 500;
    std::vector<std::size_t> image_dimensions{3, crop_height, crop_width};
    mlio::image_reader_params img_prm{
        image_frame::none, {}, image_dimensions, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);
    try {
        reader->read_example();
        FAIL() << "Invalid argument exception expected";
    }
    catch (std::invalid_argument &ex) {
        ASSERT_EQ(
            std::string(ex.what()).find(invalid_image_dimensions_exception_),
            0);
    }

    // invalid number of channel
    image_dimensions = {5, crop_height, crop_width};
    img_prm = {image_frame::none, {}, image_dimensions, false};
    reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);
    try {
        reader->read_example();
        FAIL() << "Invalid argument exception expected";
    }
    catch (std::invalid_argument &ex) {
        ASSERT_EQ(std::string(ex.what()), unsupported_channel_exception_);
    }

    // image_dimensions missing param
    image_dimensions = {crop_height, crop_width};
    img_prm = {image_frame::none, {}, image_dimensions, false};
    try {
        reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);
        FAIL() << "Invalid argument exception expected";
    }
    catch (std::invalid_argument &ex) {
        ASSERT_EQ(std::string(ex.what()),
                  missing_image_dimensions_prm_exception_);
    }
}

TEST_F(test_image_reader, test_bgr_to_rgb)
{
    size_t batch_size = 1;
    bool bgr_to_rgb = true;
    mlio::data_reader_params prm{png_dataset_, 1};
    std::size_t resize = 100;
    mlio::image_reader_params img_prm{
        image_frame::none, resize, image_dimensions_std_, bgr_to_rgb};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);

    auto exm = reader->read_example();
    auto lbl = static_cast<dense_tensor *>(exm->find_feature("value").get());
    auto image_buffer = lbl->data().as<std::uint8_t>();
    cv::Mat img{ret_img_height_, ret_img_width_, CV_8UC3, image_buffer.data()};

    ASSERT_NE(image_buffer.data(), nullptr);
    cv::Mat expected_image{get_expected_image(img0_path_png_, resize)};
    cv::Mat output;
    cv::cvtColor(expected_image, expected_image, cv::COLOR_BGR2RGB);
    cv::bitwise_xor(expected_image, img, output);
    ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
    auto schema = exm->get_schema();
    assert_schema(schema, batch_size, ret_img_3channel_);
    assert_tensor_shape(lbl, batch_size, ret_img_3channel_);
}

TEST_F(test_image_reader, test_recordio_3_channel)
{
    size_t batch_size = 1;

    mlio::data_reader_params prm{recordio_dataset_, batch_size};
    mlio::image_reader_params img_prm{
        image_frame::recordio, {}, image_dimensions_std_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);

    auto exm = reader->read_example();
    auto lbl = static_cast<dense_tensor *>(exm->find_feature("value").get());
    auto image_buffer = lbl->data().as<std::uint8_t>();
    cv::Mat img_0{
        ret_img_height_, ret_img_width_, CV_8UC3, image_buffer.data()};

    ASSERT_NE(image_buffer.data(), nullptr);
    auto schema = exm->get_schema();
    assert_schema(schema, batch_size, ret_img_3channel_);
    assert_tensor_shape(lbl, batch_size, ret_img_3channel_);
}

TEST_F(test_image_reader, test_recordio_1_channel)
{
    size_t batch_size = 1;

    mlio::data_reader_params prm{recordio_dataset_, batch_size};
    mlio::image_reader_params img_prm{
        image_frame::recordio, {}, image_dimensions_1_ch_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);

    auto exm = reader->read_example();
    auto lbl = static_cast<dense_tensor *>(exm->find_feature("value").get());
    auto image_buffer = lbl->data().as<std::uint8_t>();
    cv::Mat img_0{
        ret_img_height_, ret_img_width_, CV_8UC3, image_buffer.data()};

    ASSERT_NE(image_buffer.data(), nullptr);
    auto schema = exm->get_schema();
    assert_schema(schema, batch_size, ret_img_1channel_);
    assert_tensor_shape(lbl, batch_size, ret_img_1channel_);
}

TEST_F(test_image_reader, test_bad_batch_handling_skip)
{
    size_t batch_size = 2;
    mlio::data_reader_params prm{jpeg_dataset_,
                                 batch_size,
                                 {},
                                 {},
                                 last_batch_handling::none,
                                 bad_batch_handling::skip};

    // passing in a larger width so that crop operation on the second
    // image would fail, as required width would be greater than
    // the source image width. This would cause the batch to be skipped. Next
    // batch would only contain a single image, as there are 3 jpeg images
    // in total, and that is what we assert on.
    mlio::image_reader_params img_prm{
        image_frame::none, {}, image_dimensions_large_width_, false};
    auto reader = mlio::make_intrusive<mlio::image_reader>(prm, img_prm);
    for (auto i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        while ((exm = reader->read_example()) != nullptr) {
            auto lbl =
                static_cast<dense_tensor *>(exm->find_feature("value").get());
            auto image_buffer = lbl->data().as<std::uint8_t>();
            cv::Mat img_0{ret_img_height_,
                          ret_img_width_large_,
                          CV_8UC3,
                          image_buffer.data()};

            ASSERT_NE(image_buffer.data(), nullptr);
            cv::Mat output;
            cv::Mat expected_image =
                cv::imread(img2_path_jpg_, cv::IMREAD_COLOR);

            // crop
            int y = (expected_image.rows - (ret_img_height_)) / 2;
            int x = (expected_image.cols - (ret_img_width_large_)) / 2;
            cv::Rect roi(x, y, (ret_img_width_large_), (ret_img_height_));
            auto im2 = expected_image(roi);

            cv::bitwise_xor(im2, img_0, output);
            ASSERT_EQ(cv::countNonZero(output.reshape(1)), 0);
            auto schema = exm->get_schema();
            ASSERT_EQ(lbl->shape()[0], 1);
            ASSERT_EQ(lbl->shape()[1], ret_img_height_);
            ASSERT_EQ(lbl->shape()[2], ret_img_width_large_);
            ASSERT_EQ(lbl->shape()[3], 3);
        }
        reader->reset();
    }
    ASSERT_TRUE(true);
}

}  // namespace mlio
