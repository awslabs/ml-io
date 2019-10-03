function(enable_link_time_optimization TARGET_NAME)
    set_property(TARGET ${TARGET_NAME} PROPERTY
        INTERPROCEDURAL_OPTIMIZATION
            TRUE
    )

    # When performing LTO on macOS, mach-o object files are generated
    # under a temporary directory that gets automatically pruned by the
    # linker at the end of the build process. Thus tools such as
    # dsymutil cannot access the DWARF debug info contained in those
    # files. To make sure that the object files still exist after the
    # linkage we have to define the linker option object_path_lto that
    # points to a directory that we control.
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        get_property(
            #OUTPUT_VARIABLE
                _TARGET_TYPE
            TARGET
                ${TARGET_NAME}
            PROPERTY
                TYPE
        )

        if(_TARGET_TYPE STREQUAL "STATIC_LIBRARY")
            return()
        endif()

        set(_LTO_DIR ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/lto.d/${TARGET_NAME}/${CMAKE_CFG_INTDIR})

        add_custom_command(
            TARGET
                ${TARGET_NAME}
            PRE_BUILD
            COMMAND
                ${CMAKE_COMMAND} -E
                    make_directory ${_LTO_DIR}
            VERBATIM
        )

        # See man ld(1).
        target_link_options(${TARGET_NAME}
            PRIVATE
                LINKER:-object_path_lto ${_LTO_DIR}
        )

        set_property(DIRECTORY APPEND PROPERTY
            ADDITIONAL_MAKE_CLEAN_FILES
                ${_LTO_DIR}
        )
    endif()
endfunction()
