set(MLIO_CAN_STRIP FALSE)

if(DEFINED CMAKE_CONFIGURATION_TYPES OR NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        find_program(MLIO_OBJCOPY_EXECUTABLE objcopy)
        mark_as_advanced(MLIO_OBJCOPY_EXECUTABLE)

        if(NOT MLIO_OBJCOPY_EXECUTABLE)
            message(FATAL_ERROR "objcopy is not found!")
        endif()

        set(MLIO_CAN_STRIP TRUE)
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        find_program(MLIO_DSYMUTIL_EXECUTABLE dsymutil)
        mark_as_advanced(MLIO_DSYMUTIL_EXECUTABLE)

        if(NOT MLIO_DSYMUTIL_EXECUTABLE)
            message(FATAL_ERROR "dsymutil is not found!")
        endif()

        find_program(MLIO_STRIP_EXECUTABLE strip)
        mark_as_advanced(MLIO_STRIP_EXECUTABLE)

        if(NOT MLIO_STRIP_EXECUTABLE)
            message(FATAL_ERROR "strip is not found!")
        endif()

        set(MLIO_CAN_STRIP TRUE)
    endif()
endif()

#
# Attempt to reduce the file size of an executable or shared library.
#
# Debug:
#   Do nothing.
#
# RelWithDebInfo:
#   Move the debug sections and the static symbol table to a separate
#   file.
#
# Other:
#   Drop the debug sections and the static symbol table.
#
function(strip_symbols TARGET_NAME)
    if(NOT MLIO_CAN_STRIP)
        return()
    endif()

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

    set(_TARGET_FILE $<TARGET_FILE:${TARGET_NAME}>)

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(_SYMBOL_FILE ${_TARGET_FILE}.debug)

        add_custom_command(
            TARGET
                ${TARGET_NAME}
            POST_BUILD
            COMMAND
                $<$<CONFIG:RelWithDebInfo>:${MLIO_OBJCOPY_EXECUTABLE}$<SEMICOLON>--only-keep-debug$<SEMICOLON>${_TARGET_FILE}$<SEMICOLON>${_SYMBOL_FILE}>
            COMMAND
                $<$<NOT:$<CONFIG:Debug>>:${MLIO_OBJCOPY_EXECUTABLE}$<SEMICOLON>--strip-unneeded$<SEMICOLON>${_TARGET_FILE}>
            COMMAND
                $<$<CONFIG:RelWithDebInfo>:${MLIO_OBJCOPY_EXECUTABLE}$<SEMICOLON>--add-gnu-debuglink=${_SYMBOL_FILE}$<SEMICOLON>${_TARGET_FILE}>
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        set(_SYMBOL_FILE ${_TARGET_FILE}.dSYM)

        add_custom_command(
            TARGET
                ${TARGET_NAME}
            POST_BUILD
            COMMAND
                $<$<CONFIG:RelWithDebInfo>:${MLIO_DSYMUTIL_EXECUTABLE}$<SEMICOLON>--minimize$<SEMICOLON>-o$<SEMICOLON>${_SYMBOL_FILE}$<SEMICOLON>${_TARGET_FILE}>
            COMMAND
                $<$<NOT:$<CONFIG:Debug>>:${MLIO_STRIP_EXECUTABLE}$<SEMICOLON>$<IF:$<STREQUAL:${_TARGET_TYPE},EXECUTABLE>,-u$<SEMICOLON>-r,-x>$<SEMICOLON>${_TARGET_FILE}>
            COMMAND_EXPAND_LISTS
            VERBATIM
        )
    endif()

    install(
        FILES
            $<$<CONFIG:RelWithDebInfo>:${_SYMBOL_FILE}>
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
        COMPONENT
            devel
        OPTIONAL
    )

    set_property(DIRECTORY APPEND PROPERTY
        ADDITIONAL_MAKE_CLEAN_FILES
            $<$<CONFIG:RelWithDebInfo>:${_SYMBOL_FILE}>
    )
endfunction()
