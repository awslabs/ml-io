# ------------------------------------------------------------
# Target: mlio-test
# ------------------------------------------------------------

add_executable(mlio-test
    record_readers/test_blob_record_reader.cxx
    test_text_line_reader.cxx
    test_recordio_protobuf_reader.cxx)

target_include_directories(mlio-test
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
)

if(CMAKE_CXX_CLANG_TIDY)
    # Some GTest macros cause the following clang-tidy checks to fail.
    set_property(TARGET mlio-test APPEND
        PROPERTY
            CXX_CLANG_TIDY "-checks=-cert-err58-cpp,-cppcoreguidelines-special-member-functions"
    )
endif()

target_link_libraries(mlio-test
    PRIVATE
        GTest::GTest GTest::Main mlio
)

if(MLIO_BUILD_IMAGE_READER)
    target_sources(mlio-test
        PRIVATE
            test_image_reader.cxx
    )
    target_link_libraries(mlio-test
        PRIVATE
            opencv_core opencv_imgcodecs opencv_imgproc
    )
endif()

# ------------------------------------------------------------
# Tests
# ------------------------------------------------------------

add_test(
    NAME
        mlio-test
    COMMAND
        mlio-test
    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}/tests/mlio-test
)

set_property(TEST mlio-test APPEND
    PROPERTY
        ENVIRONMENT "UBSAN_OPTIONS=suppressions=${PROJECT_SOURCE_DIR}/UBSan.supp"
)
