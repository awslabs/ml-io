add_subdirectory(detail/protobuf)

# ------------------------------------------------------------
# Target: mlio
# ------------------------------------------------------------

add_library(mlio
    $<TARGET_OBJECTS:mlio-protobuf>
    data_stores/detail/file_util.cxx
    data_stores/compression.cxx
    data_stores/data_store.cxx
    data_stores/file.cxx
    data_stores/file_list.cxx
    data_stores/in_memory_store.cxx
    data_stores/s3_object.cxx
    data_stores/sagemaker_pipe.cxx
    detail/pathname.cxx
    detail/s3_utils.cxx
    detail/system_info.cxx
    integ/dlpack.cxx
    memory/external_memory_block.cxx
    memory/file_backed_memory_block.cxx
    memory/file_backed_memory_allocator.cxx
    memory/file_mapped_memory_block.cxx
    memory/heap_memory_allocator.cxx
    memory/heap_memory_block.cxx
    memory/memory_allocator.cxx
    memory/memory_block.cxx
    memory/memory_slice.cxx
    memory/util.cxx
    record_readers/detail/chunk_reader.cxx
    record_readers/detail/default_chunk_reader.cxx
    record_readers/detail/in_memory_chunk_reader.cxx
    record_readers/detail/recordio_header.cxx
    record_readers/detail/text_line.cxx
    record_readers/blob_record_reader.cxx
    record_readers/csv_record_reader.cxx
    record_readers/parquet_record_reader.cxx
    record_readers/record_error.cxx
    record_readers/record_reader.cxx
    record_readers/record_reader_base.cxx
    record_readers/recordio_record_reader.cxx
    record_readers/stream_record_reader.cxx
    record_readers/text_line_record_reader.cxx
    record_readers/text_record_reader.cxx
    streams/detail/iconv.cxx
    streams/detail/zlib.cxx
    streams/file_input_stream.cxx
    streams/gzip_inflate_stream.cxx
    streams/input_stream_base.cxx
    streams/input_stream.cxx
    streams/memory_input_stream.cxx
    streams/s3_input_stream.cxx
    streams/sagemaker_pipe_input_stream.cxx
    streams/stream_error.cxx
    streams/utf8_input_stream.cxx
    util/number.cxx
    util/string.cxx
    coo_tensor_builder.cxx
    cpu_array.cxx
    csv_reader.cxx
    csv_record_tokenizer.cxx
    data_reader_base.cxx
    data_reader.cxx
    data_reader_error.cxx
    data_type.cxx
    default_instance_reader.cxx
    device_array.cxx
    device.cxx
    example.cxx
    image_reader.cxx
    init.cxx
    init_aws.cxx
    instance_batch_reader.cxx
    instance_reader.cxx
    instance_reader_base.cxx
    logger.cxx
    mlio_error.cxx
    not_supported_error.cxx
    parallel_data_reader.cxx
    parser.cxx
    recordio_protobuf_reader.cxx
    s3_client.cxx
    schema.cxx
    shuffled_instance_reader.cxx
    tensor.cxx
    tensor_visitor.cxx
    text_encoding.cxx
    text_line_reader.cxx)

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
        absl::strings dlpack::dlpack fmt::fmt protobuf::libprotobuf TBB::tbb
)

target_link_libraries(mlio
    PRIVATE
        Iconv::Iconv Threads::Threads ZLIB::ZLIB
)

if(MLIO_BUILD_S3_SUPPORT)
    target_compile_definitions(mlio
        PUBLIC
            MLIO_BUILD_S3_SUPPORT
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
        include
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
