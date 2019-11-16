if(CMAKE_CXX_CLANG_TIDY)
    list(APPEND CMAKE_CXX_CLANG_TIDY "-checks=-google-build-using-namespace,-readability-else-after-return")
endif()

add_subdirectory(insights)

foreach(MLIO_CONTRIB_PY_TARGET IN LISTS MLIO_CONTRIB_PY_TARGETS)
    target_include_directories(${MLIO_CONTRIB_PY_TARGETS}
        PRIVATE
            ${PROJECT_SOURCE_DIR}/contrib/insights/include
    )

    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # Allow undefined symbols which is required for Python C
        # extensions.
        target_link_options(${MLIO_CONTRIB_PY_TARGET}
            PRIVATE
                LINKER:-undefined,dynamic_lookup
        )
    endif()
endforeach()
