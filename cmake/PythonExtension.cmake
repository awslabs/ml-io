function(add_python_extension TARGET_NAME OUTPUT_NAME)
    if(NOT TARGET pybind11::module)
        message(FATAL_ERROR "The pybind11::module target is not available.")
    endif()

    add_library(${TARGET_NAME} MODULE
        ${ARGN}
    )

    target_include_directories(${TARGET_NAME} SYSTEM
        PRIVATE
            ${PYTHON_INCLUDE_DIRS}
    )

    if(PYTHON_IS_DEBUG)
        target_compile_definitions(${TARGET_NAME}
            PRIVATE
                Py_DEBUG
        )
    endif()

    target_link_libraries(${TARGET_NAME}
        PRIVATE
            pybind11::module
    )

    if(WIN32)
        target_link_libraries(${TARGET_NAME}
            PRIVATE
                ${PYTHON_LIBRARIES}
        )
    endif()

    # Allow undefined symbols as it is required for weakly linking
    # Python C extensions.
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        target_link_options(${TARGET_NAME}
            PRIVATE
                LINKER:-z,undefs
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        target_link_options(${TARGET_NAME}
            PRIVATE
                LINKER:-undefined,dynamic_lookup
        )
    endif()

    set_target_properties(${TARGET_NAME} PROPERTIES
        # Suppress per-configuration subdirectory by using a superfluous
        # generator expression.
        LIBRARY_OUTPUT_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}/../$<0:>
        SKIP_BUILD_RPATH
            TRUE
        OUTPUT_NAME
            ${OUTPUT_NAME}
        PREFIX
            ""
        SUFFIX
            "${PYTHON_MODULE_EXTENSION}"
    )
endfunction()
