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
        20190808
    CMAKE_ARGS
        -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
        -DCMAKE_BUILD_TYPE=Release
        -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_CXX_VISIBILITY_PRESET=hidden
        -DCMAKE_VISIBILITY_INLINES_HIDDEN=TRUE
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
        -DBUILD_MOCK=FALSE
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
        -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE
        -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
        -DCMAKE_CXX_VISIBILITY_PRESET=hidden
        -DCMAKE_VISIBILITY_INLINES_HIDDEN=TRUE
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

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(TIO_TBB_URL "https://github.com/intel/tbb/releases/download/2019_U8/tbb2019_20190605oss_lin.tgz")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(TIO_TBB_URL "https://github.com/intel/tbb/releases/download/2019_U8/tbb2019_20190605oss_mac.tgz")
else()
    message(FATAL_ERROR "Intel TBB is only supported on Linux and macOS.")
endif()

ExternalProject_Add(
    #NAME
        tbb
    PREFIX
        ${CMAKE_CURRENT_BINARY_DIR}
    URL
        ${TIO_TBB_URL}
    CONFIGURE_COMMAND
        ""
    BUILD_COMMAND
        ""
    INSTALL_COMMAND
        cmake -E copy_directory <SOURCE_DIR>/tbb2019_20190605oss <INSTALL_DIR>
)
