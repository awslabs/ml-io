# ------------------------------------------------------------
# Target: mlio-py-arrow
# ------------------------------------------------------------

pybind11_add_module(mlio-py-arrow MODULE SYSTEM THIN_LTO
    arrow_buffer.cxx
    arrow_file.cxx
    module.cxx
)

target_link_libraries(mlio-py-arrow
    PUBLIC
        arrow_shared mlio::mlio
)

set_target_properties(mlio-py-arrow PROPERTIES
    # Suppress per-configuration subdirectory by using a superfluous
    # generator expression.
    LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_SOURCE_DIR}/src/mlio-py/mlio/integ$<0:>
    OUTPUT_NAME
        arrow
)

install(
    TARGETS
        mlio-py-arrow
    LIBRARY
        DESTINATION
            ${PROJECT_BINARY_DIR}/wheel/mlio/mlio/integ
        COMPONENT
            python
)

set(MLIO_PY_TARGETS ${MLIO_PY_TARGETS} mlio-py-arrow PARENT_SCOPE)
