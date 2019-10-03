# ------------------------------------------------------------
# Target: mlio-py-core
# ------------------------------------------------------------

pybind11_add_module(mlio-py-core MODULE SYSTEM THIN_LTO
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

target_link_libraries(mlio-py-core
    PRIVATE
        dlpack::dlpack fmt::fmt mlio::mlio
)

set_target_properties(mlio-py-core PROPERTIES
    # Suppress per-configuration subdirectory by using a superfluous
    # generator expression.
    LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_SOURCE_DIR}$<1:/src/mlio-py/mlio>
    OUTPUT_NAME
        core
)

set(MLIO_PY_TARGETS ${MLIO_PY_TARGETS} mlio-py-core PARENT_SCOPE)
