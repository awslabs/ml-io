# ------------------------------------------------------------
# Target: mlio-py
# ------------------------------------------------------------

add_python_extension(mlio-py _core
    data_reader.cc
    data_store.cc
    error.cc
    example.cc
    integ.cc
    logging.cc
    memory.cc
    module.cc
    py_buffer.cc
    py_device_array.cc
    py_memory_block.cc
    record_reader.cc
    s3_client.cc
    schema.cc
    stream.cc
    tensor.cc
)

target_link_libraries(mlio-py
    PRIVATE
        dlpack::dlpack fmt::fmt mlio::mlio
)
