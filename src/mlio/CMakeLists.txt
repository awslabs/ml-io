add_subdirectory(detail/protobuf)

# ------------------------------------------------------------
# Target: mlio
# ------------------------------------------------------------

add_library(mlio
    $<TARGET_OBJECTS:mlio-protobuf>
    data_stores/detail/util.cc
    data_stores/compression.cc
    data_stores/data_store.cc
    data_stores/file.cc
    data_stores/file_list.cc
    data_stores/in_memory_store.cc
    data_stores/s3_object.cc
    data_stores/sagemaker_pipe.cc
    detail/pathname.cc
    detail/s3_utils.cc
    detail/system_info.cc
    instance_readers/core_instance_reader.cc
    instance_readers/instance_reader.cc
    instance_readers/instance_reader_base.cc
    instance_readers/ranged_instance_reader.cc
    instance_readers/sampled_instance_reader.cc
    instance_readers/sharded_instance_reader.cc
    instance_readers/shuffled_instance_reader.cc
    integ/dlpack.cc
    memory/external_memory_block.cc
    memory/file_backed_memory_block.cc
    memory/file_backed_memory_allocator.cc
    memory/file_mapped_memory_block.cc
    memory/heap_memory_allocator.cc
    memory/heap_memory_block.cc
    memory/memory_allocator.cc
    memory/memory_block.cc
    memory/memory_slice.cc
    memory/util.cc
    record_readers/detail/chunk_reader.cc
    record_readers/detail/default_chunk_reader.cc
    record_readers/detail/in_memory_chunk_reader.cc
    record_readers/detail/recordio_header.cc
    record_readers/detail/text_line.cc
    record_readers/csv_record_reader.cc
    record_readers/parquet_record_reader.cc
    record_readers/record_error.cc
    record_readers/record_reader.cc
    record_readers/record_reader_base.cc
    record_readers/recordio_record_reader.cc
    record_readers/stream_record_reader.cc
    record_readers/text_line_record_reader.cc
    record_readers/text_record_reader.cc
    streams/detail/iconv.cc
    streams/detail/zlib.cc
    streams/file_input_stream.cc
    streams/gzip_inflate_stream.cc
    streams/input_stream_base.cc
    streams/input_stream.cc
    streams/memory_input_stream.cc
    streams/s3_input_stream.cc
    streams/sagemaker_pipe_input_stream.cc
    streams/stream_error.cc
    streams/utf8_input_stream.cc
    util/number.cc
    util/string.cc
    config.cc
    coo_tensor_builder.cc
    cpu_array.cc
    csv_reader.cc
    csv_record_tokenizer.cc
    data_reader_base.cc
    data_reader.cc
    data_reader_error.cc
    data_type.cc
    device_array.cc
    device.cc
    example.cc
    image_reader.cc
    init.cc
    init_aws.cc
    instance.cc
    instance_batch.cc
    instance_batch_reader.cc
    logger.cc
    mlio_error.cc
    not_supported_error.cc
    parallel_data_reader.cc
    parser.cc
    recordio_protobuf_reader.cc
    s3_client.cc
    schema.cc
    tensor.cc
    tensor_visitor.cc
    text_encoding.cc
    text_line_reader.cc)

target_include_directories(mlio
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

# We treat protoc-generated header files as system files to suppress
# non-actionable warnings.
target_include_directories(mlio SYSTEM
    PRIVATE
        ${PROJECT_BINARY_DIR}/src
)

if(MLIO_STATIC_LIB)
    target_compile_definitions(mlio
        PUBLIC
            MLIO_STATIC_LIB
    )
endif()

target_link_libraries(mlio
    PRIVATE
        absl::strings dlpack::dlpack fmt::fmt natsort::strnatcmp
        protobuf::libprotobuf TBB::tbb
)

target_link_libraries(mlio
    PRIVATE
        Iconv::Iconv Threads::Threads ZLIB::ZLIB
)

if(MLIO_BUILD_S3)
    target_compile_definitions(mlio
        PUBLIC
            MLIO_BUILD_S3
    )

    target_link_libraries(mlio
        PRIVATE
            aws-cpp-sdk-core aws-cpp-sdk-s3
    )
endif()

if(MLIO_BUILD_IMAGE_READER)
    target_compile_definitions(mlio
        PUBLIC
            MLIO_BUILD_IMAGE_READER
     )

    target_link_libraries(mlio
        PRIVATE
            opencv_core opencv_imgcodecs opencv_imgproc)
endif()

set_target_properties(mlio PROPERTIES
    VERSION
        ${PROJECT_VERSION}
    SOVERSION
        ${PROJECT_VERSION_MAJOR}
)

if(MLIO_ENABLE_LTO)
    set_object_path_lto(mlio)
endif()

install(
    TARGETS
        mlio
    EXPORT
        mlio-targets
    RUNTIME
        DESTINATION
            ${CMAKE_INSTALL_BINDIR}
        COMPONENT
            runtime
    LIBRARY
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
        COMPONENT
            runtime
    ARCHIVE
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
        COMPONENT
            devel
    INCLUDES DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)

install(
    FILES
        ${PROJECT_SOURCE_DIR}/include/mlio.h
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT
        devel
)

install(
    DIRECTORY
        ${PROJECT_SOURCE_DIR}/include/mlio
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
    COMPONENT
        devel
)
