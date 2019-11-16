# ------------------------------------------------------------
# Target: mlio-contrib-insights
# ------------------------------------------------------------

pybind11_add_module(mlio-contrib-insights MODULE SYSTEM THIN_LTO
    src/utils.cxx
    src/insights.cxx
    src/module.cxx
    src/column_analyzer.cxx
    src/hll/xxhash.c
    src/hll/hyperloglog.cxx
)

target_link_libraries(mlio-contrib-insights
    PRIVATE
        mlio::mlio
)

set_target_properties(mlio-contrib-insights PROPERTIES
    # Suppress per-configuration subdirectory by using a superfluous
    # generator expression.
    LIBRARY_OUTPUT_DIRECTORY
        ${PROJECT_SOURCE_DIR}/src/mlio-py/mlio/contrib$<0:>
    OUTPUT_NAME
        insights
)

install(
    TARGETS
        mlio-contrib-insights
    LIBRARY
        DESTINATION
            ${PROJECT_BINARY_DIR}/wheel/mlio/insights
        COMPONENT
            python
)

set(MLIO_CONTRIB_PY_TARGETS ${MLIO_CONTRIB_PY_TARGETS} mlio-contrib-insights PARENT_SCOPE)
