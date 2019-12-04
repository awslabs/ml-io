# ------------------------------------------------------------
# Target: mlio-py-core
# ------------------------------------------------------------

add_python_extension(mlio-py core
    data_readers.cxx
    data_stores.cxx
    example.cxx
    exceptions.cxx
    integ.cxx
    logging.cxx
    memory.cxx
    module.cxx
    py_buffer.cxx
    py_device_array.cxx
    py_memory_block.cxx
    record_readers.cxx
    schema.cxx
    streams.cxx
    tensor.cxx
)

target_link_libraries(mlio-py
    PRIVATE
        dlpack::dlpack fmt::fmt mlio::mlio
)
