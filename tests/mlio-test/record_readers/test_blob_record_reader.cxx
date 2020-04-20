#include <gtest/gtest.h>
#include <mlio.h>

namespace mlio {

class test_blob_record_reader : public ::testing::Test {
protected:
    test_blob_record_reader() = default;

    ~test_blob_record_reader() override;

protected:
    std::string const file_path_ = "../resources/test.txt";
    intrusive_ptr<file> data_store = mlio::make_intrusive<mlio::file>(file_path_);
};

test_blob_record_reader::~test_blob_record_reader() = default;

TEST_F(test_blob_record_reader, test_read_record)
{
    auto strm = data_store->open_read();
    auto expected_size = strm->size();
    auto rdr = make_intrusive<blob_record_reader>(strm);

    auto record_1 = rdr->read_record();
    auto record_2 = rdr->read_record();

    ASSERT_EQ(record_1->kind(), record_kind::complete);
    ASSERT_EQ(record_1->payload().size(), expected_size);
    ASSERT_NE(record_1->payload().data(), nullptr);
    ASSERT_EQ(record_2.has_value(), false);
    auto unread_stream = strm->read(strm->size());
    ASSERT_EQ(unread_stream.begin(), unread_stream.end());
}

TEST_F(test_blob_record_reader, test_peek_record)
{
    auto strm = data_store->open_read();
    auto expected_size = strm->size();
    auto rdr = make_intrusive<blob_record_reader>(strm);

    auto record_1 = rdr->peek_record();
    auto record_2 = rdr->peek_record();

    ASSERT_EQ(record_1->kind(), record_kind::complete);
    ASSERT_EQ(record_1->payload().size(), expected_size);
    ASSERT_NE(record_1->payload().data(), nullptr);
    ASSERT_EQ(record_2.has_value(), true);
    auto unread_stream = strm->read(strm->size());
    ASSERT_EQ(unread_stream.begin(), unread_stream.end());
}

}  // namespace mlio
