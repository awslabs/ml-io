if(CMAKE_CXX_CLANG_TIDY)
    list(APPEND CMAKE_CXX_CLANG_TIDY "-checks=-google-build-using-namespace,-readability-else-after-return")
endif()

add_subdirectory(mlio/core)

if(MLIO_INCLUDE_ARROW_INTEGRATION)
    add_subdirectory(mlio/integ/arrow)
endif()

foreach(MLIO_PY_TARGET IN LISTS MLIO_PY_TARGETS)
    target_include_directories(${MLIO_PY_TARGET}
        PRIVATE
            $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/mlio-py/mlio>
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # Allow undefined symbols which is required for Python C
        # extensions.
        target_link_options(${MLIO_PY_TARGET}
            PRIVATE
                LINKER:-undefined,dynamic_lookup
        )
    endif()
endforeach()
