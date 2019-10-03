add_subdirectory(detail/protobuf)

# ------------------------------------------------------------
# Target: mlio
# ------------------------------------------------------------

add_library(mlio
    data_stores/detail/file_util.cxx
    data_stores/compression.cxx
    data_stores/data_store.cxx
    data_stores/file.cxx
    data_stores/file_hierarchy.cxx
    data_stores/in_memory_store.cxx
    data_stores/sagemaker_pipe.cxx
    detail/pathname.cxx
    integ/dlpack.cxx
    memory/external_memory_block.cxx
    memory/file_backed_memory_allocator.cxx
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
    record_readers/corrupt_record_error.cxx
    record_readers/csv_record_reader.cxx
    record_readers/parquet_record_reader.cxx
    record_readers/recordio_record_reader.cxx
    record_readers/record_reader_base.cxx
    record_readers/record_reader.cxx
    record_readers/stream_record_reader.cxx
    record_readers/text_line_record_reader.cxx
    record_readers/text_record_reader.cxx
    streams/detail/iconv.cxx
    streams/detail/zlib.cxx
    streams/gzip_inflate_stream.cxx
    streams/input_stream_base.cxx
    streams/input_stream.cxx
    streams/memory_input_stream.cxx
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
    init.cxx
    instance_batch_reader.cxx
    instance_reader.cxx
    instance_reader_base.cxx
    logger.cxx
    not_supported_error.cxx
    parallel_data_reader.cxx
    parser.cxx
    recordio_protobuf_reader.cxx
    schema.cxx
    shuffled_instance_reader.cxx
    tensor.cxx
    tensor_visitor.cxx
    text_encoding.cxx
)

if(WIN32)
    # TODO: Windows Support
else()
    target_sources(mlio
        PRIVATE
            platform/posix/data_stores/file_hierarchy.cxx
            platform/posix/detail/system_info.cxx
            platform/posix/memory/file_backed_memory_block.cxx
            platform/posix/memory/file_mapped_memory_block.cxx
            platform/posix/streams/file_input_stream.cxx
            platform/posix/streams/sagemaker_pipe_input_stream.cxx
    )
endif()

target_include_directories(mlio
    PUBLIC
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

set_target_properties(mlio PROPERTIES
    VERSION
        ${PROJECT_VERSION}
    SOVERSION
        ${PROJECT_VERSION_MAJOR}
)

if(MLIO_STATIC_LIB)
    target_compile_definitions(mlio
        PUBLIC
            MLIO_STATIC_LIB
    )
endif()

target_link_libraries(mlio
    PRIVATE
        absl::strings dlpack::dlpack fmt::fmt TBB::tbb mlio-protobuf
)

target_link_libraries(mlio
    PRIVATE
        Iconv::Iconv Threads::Threads ZLIB::ZLIB
)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # We disallow undefined ELF symbols. Ideally this option should be
    # applied globally. However Python C extensions rely on resolving
    # undefined Python C API symbols at runtime. Unfortunately the
    # opposite -z,undefs option is first introduced with the GNU linker
    # v2.30 which is not available in all distributions we support.
    target_link_options(mlio
        PRIVATE
            LINKER:-z,defs
    )
endif()

if(MLIO_ENABLE_LTO)
    enable_link_time_optimization(mlio)
endif()

strip_symbols(mlio)

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

if(WIN32 AND MSVC AND NOT MLIO_STATIC_LIB)
  install(
      FILES
          $<TARGET_PDB_FILE:mlio>
      DESTINATION
          ${CMAKE_INSTALL_BINDIR}
      COMPONENT
          devel
      OPTIONAL
  )
endif()
