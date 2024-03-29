cmake_minimum_required(VERSION 3.13)

project(mlio-third-party LANGUAGES NONE)

set(CMAKE_CXX_STANDARD 17)

set(_CFLAGS   "-fPIC -fvisibility=hidden -fvisibility-inlines-hidden -std=c99")
set(_CXXFLAGS "-fPIC -fvisibility=hidden -fvisibility-inlines-hidden -std=c++17")

include(ExternalProject)

ExternalProject_Add(
    #NAME
        absl
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/abseil/abseil-cpp.git
    GIT_TAG
        20210324.2
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_CXX_VISIBILITY_PRESET=hidden
        -DCMAKE_VISIBILITY_INLINES_HIDDEN=ON
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    #NAME
        awssdk
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/aws/aws-sdk-cpp.git
    GIT_TAG
        1.7.290
    CMAKE_ARGS
        -DBUILD_ONLY=s3
        -DCPP_STANDARD=17
        -DENABLE_TESTING=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_VISIBILITY_INLINES_HIDDEN=ON
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    #NAME
        dlpack
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/dmlc/dlpack.git
    GIT_TAG
        0acb731e0e43d15deee27b66f10e4c5b4e667913
    CMAKE_ARGS
        -DBUILD_MOCK=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    #NAME
        fmt
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/fmtlib/fmt.git
    GIT_TAG
        6.0.0
    GIT_SHALLOW
        TRUE
    CMAKE_ARGS
        -DFMT_DOC=OFF
        -DFMT_TEST=OFF
        -DBUILD_SHARED_LIBS=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_CXX_VISIBILITY_PRESET=hidden
        -DCMAKE_VISIBILITY_INLINES_HIDDEN=ON
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    #NAME
        gtest
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/google/googletest.git
    GIT_TAG
        release-1.8.1
    GIT_SHALLOW
        TRUE
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    #NAME
        natsort
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    DOWNLOAD_COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/natsort <SOURCE_DIR>
    PATCH_COMMAND
        patch < strnatcmp.h.patch
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

include(ProcessorCount)

ProcessorCount(N)

ExternalProject_Add(
    #NAME
        protobuf
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/google/protobuf.git
    GIT_TAG
        v3.8.0
    GIT_SHALLOW
        TRUE
    CONFIGURE_COMMAND
        ./autogen.sh
    COMMAND
        ./configure "CFLAGS=${_CFLAGS}" "CXXFLAGS=${_CXXFLAGS}" --disable-shared --prefix=<INSTALL_DIR>
    BUILD_COMMAND
        make -j ${N}
    COMMAND
        make check
    COMMAND
        make install
    BUILD_IN_SOURCE
        TRUE
)

ExternalProject_Add(
    #NAME
        pybind11
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/pybind/pybind11.git
    GIT_TAG
        v2.4.2
    GIT_SHALLOW
        TRUE
    CMAKE_ARGS
        -DPYBIND11_TEST=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
)

ExternalProject_Add(
    #NAME
        tbb
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    GIT_REPOSITORY
        https://github.com/wjakob/tbb.git
    GIT_TAG
        ddbe45cd3ad89df9a84cd77013d5898fc48b8e89
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_POSITION_INDEPENDENT_CODE=ON
        -DBUILD_SHARED_LIBS=OFF
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
)
