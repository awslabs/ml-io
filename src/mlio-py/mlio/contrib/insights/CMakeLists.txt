# ------------------------------------------------------------
# Target: hyperloglog
# ------------------------------------------------------------

add_library(hyperloglog OBJECT
    hll/hyperloglog.cc
    hll/xxhash.c
)

set_target_properties(hyperloglog PROPERTIES
    CXX_CLANG_TIDY
        ""
    CXX_INCLUDE_WHAT_YOU_USE
        ""
)

target_compile_options(hyperloglog
    PRIVATE
        -w
)

# ------------------------------------------------------------
# Target: mlio-contrib
# ------------------------------------------------------------

add_python_extension(mlio-contrib insights
    $<TARGET_OBJECTS:hyperloglog>
    column_analyzer.cc
    utils.cc
    module.cc
)

target_link_libraries(mlio-contrib
    PRIVATE
        hyperloglog mlio::mlio
)

target_include_directories(mlio-contrib SYSTEM
    PRIVATE
        hll
)
