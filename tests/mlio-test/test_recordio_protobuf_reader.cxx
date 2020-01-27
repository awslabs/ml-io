#include <gtest/gtest.h>
#include <mlio.h>

namespace mlio {

class test_recordio_protobuf_reader : public ::testing::Test {
protected:
    test_recordio_protobuf_reader() = default;

    ~test_recordio_protobuf_reader() override;

protected:
    std::string const resources_path_ = "../resources/recordio/";
    std::string const complete_records_path_ = resources_path_ + "complete_records.pr";
    std::string const split_records_path_ = resources_path_ + "split_records.pr";
    std::string const corrupt_split_records_path_ = resources_path_ + "corrupted_split_records.pr";
};

test_recordio_protobuf_reader::~test_recordio_protobuf_reader() = default;

TEST_F(test_recordio_protobuf_reader, test_complete_records_path)
{
    mlio::data_reader_params prm{};
    prm.dataset.emplace_back(mlio::make_intrusive<mlio::file>(complete_records_path_));
    prm.batch_size = 1;

    auto reader = mlio::make_intrusive<mlio::recordio_protobuf_reader>(prm);
    for (int i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        while ((exm = reader->read_example()) != nullptr) {
        }
        reader->reset();
    }

    EXPECT_TRUE(true);
}

TEST_F(test_recordio_protobuf_reader, test_split_records_path)
{
    mlio::initialize();
    mlio::data_reader_params prm{};
    prm.dataset.emplace_back(mlio::make_intrusive<mlio::file>(split_records_path_));
    prm.batch_size = 1;

    auto reader = mlio::make_intrusive<mlio::recordio_protobuf_reader>(prm);
    for (int i = 0; i < 2; i++) {
        mlio::intrusive_ptr<mlio::example> exm;
        while ((exm = reader->read_example()) != nullptr) {
        }
        reader->reset();
    }

    EXPECT_TRUE(true);
}

TEST_F(test_recordio_protobuf_reader, test_corrupt_records_patH)
{
    mlio::initialize();
    // Check that the third record is corrupt.
    mlio::data_reader_params prm{};
    prm.dataset.emplace_back(mlio::make_intrusive<mlio::file>(corrupt_split_records_path_ ));
    prm.batch_size = 10;
    prm.num_prefetched_batches = 1;

    std::string error_substring = "The record 3 in the data store";

    auto reader = mlio::make_intrusive<mlio::recordio_protobuf_reader>(prm);
    mlio::intrusive_ptr<mlio::example> exm;
    
    // Try to read batch, should fail due to corrupt record
    try {
        exm = reader->read_example();
        FAIL() << "Expected corrupt error exception on 3rd record.";
    }
    catch (data_reader_error const &corrupt_record_err) {
        EXPECT_TRUE(std::string(corrupt_record_err.what()).find(error_substring) != std::string::npos) 
            << "Error thrown:" + std::string(corrupt_record_err.what());    
    }
    catch(...) {
        FAIL() << "Expected corrupt error exception, not a different error.";
    }

    // Try to read batch, should fail again
    try {
        exm = reader->read_example();
        FAIL() << "Expected corrupt error exception on 3rd record.";
    }
    catch (data_reader_error const &corrupt_record_err) {
        EXPECT_TRUE(std::string(corrupt_record_err.what()).find(error_substring) != std::string::npos) 
            << "Error thrown:" + std::string(corrupt_record_err.what());    
    }
    catch(...) {
        FAIL() << "Expected corrupt error exception, not a different error.";
    }

    // Reset reader, expecting same results.
    reader->reset();
    
    // Try to read batch, should fail again
    try {
        exm = reader->read_example();
        FAIL() << "Expected corrupt error exception on 3rd record.";
    }
    catch (data_reader_error const &corrupt_record_err) {
        EXPECT_TRUE(std::string(corrupt_record_err.what()).find(error_substring) != std::string::npos) 
            << "Error thrown:" + std::string(corrupt_record_err.what());    
    }
    catch(...) {
        FAIL() << "Expected corrupt error exception, not a different error.";
    }
}

}
