# ------------------------------------------------------------
# Target: mlio-py
# ------------------------------------------------------------

add_python_extension(mlio-py _core
    data_reader.cxx
    data_store.cxx
    error.cxx
    example.cxx
    integ.cxx
    logging.cxx
    memory.cxx
    module.cxx
    py_buffer.cxx
    py_device_array.cxx
    py_memory_block.cxx
    record_reader.cxx
    schema.cxx
    stream.cxx
    tensor.cxx
)

target_link_libraries(mlio-py
    PRIVATE
        dlpack::dlpack fmt::fmt mlio::mlio
)
