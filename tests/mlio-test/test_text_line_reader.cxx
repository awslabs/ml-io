#include <gtest/gtest.h>
#include <mlio.h>

namespace mlio {

class test_text_line_reader : public ::testing::Test {
protected:
    test_text_line_reader() = default;

    ~test_text_line_reader() override;

protected:
    std::string const expected_line_1_ = "this is line 1";
    std::string const expected_line_2_ = "this is line 2";
    std::string const expected_line_3_ = "this is line 3";
    std::string const file_path_ = "../resources/test.txt";
};

test_text_line_reader::~test_text_line_reader() = default;

TEST_F(test_text_line_reader, test_text_line_reader_happy_path)
{
    mlio::data_reader_params prm{};
    prm.dataset.emplace_back(mlio::make_intrusive<mlio::file>(file_path_));
    prm.batch_size = 3;

    auto reader = mlio::make_intrusive<mlio::text_line_reader>(prm);
    for (auto i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        while ((exm = reader->read_example()) != nullptr) {
            auto lbl =
                static_cast<dense_tensor *>(exm->find_feature("value").get());
            auto strings = lbl->data().as<std::string>();
            EXPECT_EQ(strings[0], expected_line_1_);
            EXPECT_EQ(strings[1], expected_line_2_);
            EXPECT_EQ(strings[2], expected_line_3_);
        }
        reader->reset();
    }

    EXPECT_TRUE(true);
}

TEST_F(test_text_line_reader,
       test_text_line_reader_batch_greater_than_features)
{
    mlio::data_reader_params prm{};
    prm.dataset.emplace_back(mlio::make_intrusive<mlio::file>(file_path_));
    prm.batch_size = 5;

    auto reader = mlio::make_intrusive<mlio::text_line_reader>(prm);
    for (auto i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        while ((exm = reader->read_example()) != nullptr) {
            auto lbl =
                static_cast<dense_tensor *>(exm->find_feature("value").get());
            auto strings = lbl->data().as<std::string>();
            EXPECT_EQ(strings[0], expected_line_1_);
            EXPECT_EQ(strings[1], expected_line_2_);
            EXPECT_EQ(strings[2], expected_line_3_);
        }
        reader->reset();
    }
    EXPECT_TRUE(true);
}

TEST_F(test_text_line_reader, test_text_line_reader_batch_less_than_features)
{
    mlio::data_reader_params prm{};
    prm.dataset.emplace_back(mlio::make_intrusive<mlio::file>(file_path_));
    prm.batch_size = 2;

    auto reader = mlio::make_intrusive<mlio::text_line_reader>(prm);
    for (auto i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        int batch_no = 1;
        while ((exm = reader->read_example()) != nullptr) {
            auto lbl =
                static_cast<dense_tensor *>(exm->find_feature("value").get());
            auto strings = lbl->data().as<std::string>();
            if (batch_no == 1) {
                EXPECT_EQ(strings[0], expected_line_1_);
                EXPECT_EQ(strings[1], expected_line_2_);
                batch_no++;
            }
            else if (batch_no == 2) {
                EXPECT_EQ(strings[0], expected_line_3_);
            }
            else {
                throw std::runtime_error("invalid test setup");
            }
        }
        reader->reset();
    }
    EXPECT_TRUE(true);
}

}  // namespace mlio
