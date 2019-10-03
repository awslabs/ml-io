# ------------------------------------------------------------
# Target: mlio-test
# ------------------------------------------------------------

add_executable(mlio-test
    foo.cxx
)

target_include_directories(mlio-test
    PRIVATE
        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/tests>
)

if(CMAKE_CXX_CLANG_TIDY)
    # Some GTest macros cause the following clang-tidy checks to fail.
    set_property(TARGET mlio-test APPEND
        PROPERTY
            CXX_CLANG_TIDY "-checks=-cert-err58-cpp,-cppcoreguidelines-special-member-functions"
    )
endif()

target_link_libraries(mlio-test
    PRIVATE
        GTest::GTest GTest::Main mlio
)

# ------------------------------------------------------------
# Tests
# ------------------------------------------------------------

add_test(
    NAME
        mlio-test
    COMMAND
        mlio-test
    WORKING_DIRECTORY
        ${PROJECT_SOURCE_DIR}/tests/mlio-test
)
