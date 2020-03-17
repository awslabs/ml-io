# Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License"). You
# may not use this file except in compliance with the License. A copy of
# the License is located at
#
#      http://aws.amazon.com/apache2.0/
#
# or in the "license" file accompanying this file. This file is
# distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF
# ANY KIND, either express or implied. See the License for the specific
# language governing permissions and limitations under the License.

cmake_minimum_required(VERSION 3.13)

# ------------------------------------------------------------
# Lookup Paths
# ------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

# ------------------------------------------------------------
# Project
# ------------------------------------------------------------

project(ML-IO VERSION 0.4.1 LANGUAGES C CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE RelWithDebInfo)
endif()

# ------------------------------------------------------------
# Cache Entries
# ------------------------------------------------------------

include(CMakeDependentOption)

option(BUILD_SHARED_LIBS "If set, builds as a shared library." ON)

option(MLIO_INCLUDE_LIB "If set, generates build target for the library." ON)
option(MLIO_INCLUDE_CONTRIB "If set, generates build target for the contrib library.")
option(MLIO_INCLUDE_PYTHON_EXTENSION "If set, generates build target for the Python C extension.")
option(MLIO_BUILD_IMAGE_READER "If set, builds the image reader." OFF)

cmake_dependent_option(
    MLIO_INCLUDE_ARROW_INTEGRATION "If set, generates build target for the Apache Arrow integration." OFF
    MLIO_INCLUDE_PYTHON_EXTENSION OFF
)

option(MLIO_INCLUDE_TESTS "If set, generates build target for the tests." ON)
option(MLIO_INCLUDE_DOC "If set, generates build target for the documentation.")

option(MLIO_BUILD_S3_SUPPORT "If set, builds with Amazon S3 support.")

option(MLIO_TREAT_WARNINGS_AS_ERRORS "If set, treats compilation warnings as errors.")

# Optimizations
option(MLIO_BUILD_FOR_NATIVE_ARCH "If set, builds for the processor type of the compiling machine.")
option(MLIO_ENABLE_LTO "If set, enables link time optimization." ON)

# Sanitizers
option(MLIO_ENABLE_ASAN  "If set, enables ASan.")
option(MLIO_ENABLE_UBSAN "If set, enables UBSan.")
option(MLIO_ENABLE_TSAN  "If set, enables TSan.")

# Static Analyzers
option(MLIO_USE_CLANG_TIDY "If set, uses clang-tidy as static analyzer.")
option(MLIO_USE_IWYU "If set, uses include-what-you-use.")

# ------------------------------------------------------------
# Global Settings
# ------------------------------------------------------------

include(LTO)
include(GNUInstallDirs)

add_compile_definitions(MLIO_COMPILE_LIB)

if(BUILD_SHARED_LIBS)
    set(MLIO_STATIC_LIB OFF)
else()
    set(MLIO_STATIC_LIB ON)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_C_VISIBILITY_PRESET hidden)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    set(CMAKE_DEBUG_POSTFIX d)
endif()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if(MLIO_ENABLE_LTO)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
endif()

if(MLIO_INCLUDE_TESTS)
    enable_testing()
endif()

# Several dependencies transitively use pthreads library. We make sure
# that they use the -pthread flag if available.
set(THREADS_PREFER_PTHREAD_FLAG TRUE)

# ------------------------------------------------------------
# Global Compiler and Linker Options
# ------------------------------------------------------------

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -fasynchronous-unwind-tables
        -fstack-protector-strong
    )

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(
            -Wall
            -Wcast-align
            -Wconversion
            -Wdouble-promotion
            -Wextra
            -Wfloat-equal
            -Wformat=2
            -Winit-self
            -Wlogical-op
            -Wno-unknown-pragmas
            -Wpointer-arith
            -Wshadow
            -Wsign-conversion
            -Wswitch-enum
            -Wunused
            $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
            $<$<COMPILE_LANGUAGE:CXX>:-Wold-style-cast>
            $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
            $<$<COMPILE_LANGUAGE:CXX>:-Wuseless-cast>
        )

        # See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=57709.
        # See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61489.
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 5)
            add_compile_options(-Wno-shadow -Wno-missing-field-initializers)
        endif()

        # See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=43407.
        if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6)
            add_compile_options(-Wno-attributes)
        endif()

        add_compile_definitions(_GLIBCXX_ASSERTIONS)
    else()
        add_compile_options(
            -fsized-deallocation
            -Weverything
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-exit-time-destructors
            -Wno-global-constructors
            -Wno-padded
            -Wno-shadow-uncaptured-local
        )
    endif()

    if(MLIO_TREAT_WARNINGS_AS_ERRORS)
        add_compile_options(-Werror)
    endif()

    if(MLIO_BUILD_FOR_NATIVE_ARCH)
        add_compile_options(-march=native -mtune=native)
    endif()

    add_compile_options(
        $<$<NOT:$<CONFIG:Debug>>:-U_FORTIFY_SOURCE>
        $<$<NOT:$<CONFIG:Debug>>:-D_FORTIFY_SOURCE=2>
    )
else()
    message(FATAL_ERROR "Unsupported compiler!")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    add_link_options(
        LINKER:--as-needed
        LINKER:--build-id=sha1
        LINKER:-z,noexecstack
        LINKER:-z,now
        LINKER:-z,relro
        LINKER:-z,defs
    )

    if(MLIO_TREAT_WARNINGS_AS_ERRORS)
        add_link_options(
            LINKER:--fatal-warnings
        )
    endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    add_link_options(
        LINKER:-bind_at_load
        LINKER:-undefined,error
    )

    if(MLIO_TREAT_WARNINGS_AS_ERRORS)
        add_link_options(
            LINKER:--fatal-warnings
        )
    endif()
endif()

# ------------------------------------------------------------
# Sanitizers
# ------------------------------------------------------------

if(MLIO_ENABLE_ASAN OR MLIO_ENABLE_UBSAN OR MLIO_ENABLE_TSAN)
    add_compile_options(-fno-omit-frame-pointer)
endif()

if(MLIO_ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        add_compile_options(-D_GLIBCXX_SANITIZE_VECTOR)
    endif()

    add_compile_options(-fsanitize=address)
    add_link_options   (-fsanitize=address)
endif()

if(MLIO_ENABLE_UBSAN)
    add_compile_options(-fsanitize=undefined)
    add_link_options   (-fsanitize=undefined)
endif()

if(MLIO_ENABLE_TSAN)
    add_compile_options(-fsanitize=thread)
    add_link_options   (-fsanitize=thread)
endif()

# ------------------------------------------------------------
# Static Analyzers
# ------------------------------------------------------------

if(MLIO_USE_CLANG_TIDY)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "clang-tidy is only supported with Clang!")
    endif()

    find_program(MLIO_CLANG_TIDY NAMES clang-tidy)
    if(NOT MLIO_CLANG_TIDY)
        message(FATAL_ERROR "clang-tidy is not found!")
    endif()

    set(CMAKE_CXX_CLANG_TIDY ${MLIO_CLANG_TIDY})
endif()

if(MLIO_USE_IWYU)
    if(NOT CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "include-what-you-use is only supported with Clang!")
    endif()

    find_program(MLIO_IWYU NAMES include-what-you-use)
    if(NOT MLIO_IWYU)
        message(FATAL_ERROR "include-what-you-use is not found!")
    endif()

    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${MLIO_IWYU};-Xiwyu;--mapping_file=${PROJECT_SOURCE_DIR}/iwyu.imp)
endif()

# ------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------

find_package(dlpack 0.1.0 REQUIRED CONFIG)
find_package(fmt 6.0 REQUIRED CONFIG)

if(MLIO_INCLUDE_LIB)
    find_package(absl REQUIRED CONFIG)
    find_package(Iconv REQUIRED)
    find_package(Protobuf 3.8 REQUIRED)
    find_package(TBB 2019.0 REQUIRED CONFIG COMPONENTS tbb)
    find_package(Threads REQUIRED)
    find_package(ZLIB REQUIRED)

    if(MLIO_BUILD_S3_SUPPORT)
        find_package(AWSSDK 1.7 REQUIRED CONFIG COMPONENTS s3)
    endif()

    if(MLIO_BUILD_IMAGE_READER)
        find_package(OpenCV 4.0 REQUIRED core imgproc imgcodecs)
    endif()

    if(MLIO_INCLUDE_TESTS)
        find_package(GTest REQUIRED)
    endif()
else()
    find_package(mlio ${PROJECT_VERSION} REQUIRED CONFIG)
endif()

if(MLIO_INCLUDE_PYTHON_EXTENSION)
    # The minimum required Python version to compile the C extensions.
    set(PYBIND11_PYTHON_VERSION 3.6)
    find_package(pybind11 2.4.2 REQUIRED CONFIG)

    if(MLIO_INCLUDE_ARROW_INTEGRATION)
        find_package(Arrow 0.16.0 REQUIRED CONFIG)
    endif()

    include(PythonExtension)
endif()

if(MLIO_INCLUDE_DOC)
    find_package(Doxygen 1.8 REQUIRED)
endif()

# ------------------------------------------------------------
# Targets
# ------------------------------------------------------------

if(MLIO_INCLUDE_LIB)
    add_subdirectory(src/mlio)

    add_library(mlio::mlio ALIAS mlio)

    if(MLIO_INCLUDE_TESTS)
        add_subdirectory(tests/mlio-test)
    endif()
endif()

if(MLIO_INCLUDE_PYTHON_EXTENSION)
    add_subdirectory(src/mlio-py)
endif()

# ------------------------------------------------------------
# Documentation
# ------------------------------------------------------------

if(MLIO_INCLUDE_DOC)
    set(DOXYGEN_OUTPUT_DIRECTORY    ${PROJECT_BINARY_DIR}/doc)
    set(DOXYGEN_EXTRACT_ALL         YES)
    set(DOXYGEN_QUIET               YES)
    set(DOXYGEN_GENERATE_HTML       YES)
    set(DOXYGEN_GENERATE_XML        YES)
    set(DOXYGEN_JAVADOC_AUTOBRIEF   YES)
    set(DOXYGEN_EXCLUDE_PATTERNS    platform;config.h)
    set(DOXYGEN_EXCLUDE_SYMBOLS     std;mlio::*::detail;data_type_traits<*>;operator<<)

    doxygen_add_docs(
        #TARGET
            mlio-doc
        #SOURCE_DIR
            include/mlio
    )

    install(
        DIRECTORY
            ${PROJECT_BINARY_DIR}/doc/
        DESTINATION
            ${CMAKE_INSTALL_DATAROOTDIR}/doc/mlio-${PROJECT_VERSION}
        COMPONENT
            devel
        OPTIONAL
    )

    set_property(DIRECTORY APPEND PROPERTY
        ADDITIONAL_MAKE_CLEAN_FILES
            ${PROJECT_BINARY_DIR}/doc/.
    )
endif()

# ------------------------------------------------------------
# Package Config
# ------------------------------------------------------------

if(MLIO_INCLUDE_LIB)
    include(CMakePackageConfigHelpers)

    configure_package_config_file(
        #INPUT
            ${PROJECT_SOURCE_DIR}/src/mlio-config.cmake.in
        #OUTPUT
            ${PROJECT_BINARY_DIR}/lib/cmake/mlio/mlio-config.cmake
        INSTALL_DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/mlio-${PROJECT_VERSION}
        NO_SET_AND_CHECK_MACRO
    )

    write_basic_package_version_file(
        #OUTPUT
            ${PROJECT_BINARY_DIR}/lib/cmake/mlio/mlio-config-version.cmake
        VERSION
            ${PROJECT_VERSION}
        COMPATIBILITY
            AnyNewerVersion
    )

    install(
        FILES
            ${PROJECT_BINARY_DIR}/lib/cmake/mlio/mlio-config.cmake
            ${PROJECT_BINARY_DIR}/lib/cmake/mlio/mlio-config-version.cmake
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/mlio-${PROJECT_VERSION}
        COMPONENT
            devel
    )

    install(
        EXPORT
            mlio-targets
        FILE
            mlio-targets.cmake
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}/cmake/mlio-${PROJECT_VERSION}
        COMPONENT
            devel
        NAMESPACE
            mlio::
    )

    export(
        EXPORT
            mlio-targets
        FILE
            ${PROJECT_BINARY_DIR}/lib/cmake/mlio/mlio-targets.cmake
        NAMESPACE
            mlio::
    )
endif()
