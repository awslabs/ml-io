cmake_minimum_required(VERSION 3.13)

project(natsort LANGUAGES C)

include(GNUInstallDirs)

add_library(strnatcmp STATIC
    strnatcmp.c
)

set_target_properties(strnatcmp PROPERTIES
    C_VISIBILITY_PRESET
        hidden
)

install(
    EXPORT
        natsort-targets
    FILE
        natsort-targets.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/natsort
    NAMESPACE
        natsort::
)

install(
    FILES
        ${PROJECT_SOURCE_DIR}/natsort-config.cmake
    DESTINATION
        ${CMAKE_INSTALL_LIBDIR}/cmake/natsort
)

install(
    TARGETS
        strnatcmp
    EXPORT
        natsort-targets
    ARCHIVE
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
)

install(
    FILES
        ${PROJECT_SOURCE_DIR}/strnatcmp.h
    DESTINATION
        ${CMAKE_INSTALL_INCLUDEDIR}
)
