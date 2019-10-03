#!/usr/bin/env bash

set -o errexit

if [[ $HOST =~ .*linux.* ]]; then
    _toolchain_file=$RECIPE_DIR/linux-toolchain.cmake
fi

# Build the dependencies first.
_dependencies_dir=$SRC_DIR/third-party/build/$(uname -s)-$(uname -m)-conda

mkdir -p "$_dependencies_dir" && cd "$_"

cmake -GNinja\
      -DCMAKE_TOOLCHAIN_FILE="$_toolchain_file"\
      "$SRC_DIR/third-party"

for _dep in absl dlpack fmt gtest pybind11; do
    cmake --build . --target $_dep
done

# Now build libmlio.
mkdir -p "$SRC_DIR/build/conda" && cd "$_"

cmake -GNinja\
      -DCMAKE_INSTALL_PREFIX="$PREFIX"\
      -DCMAKE_TOOLCHAIN_FILE="$_toolchain_file"\
      -DCMAKE_FIND_ROOT_PATH="$_dependencies_dir"\
      -DIconv_IS_BUILT_IN=FALSE\
      -DMLIO_INCLUDE_DOC=TRUE\
      "$SRC_DIR"

cmake --build .
cmake --build . --target mlio-doc
cmake --build . --target test

# Install only the runtime components.
cmake -DCOMPONENT=runtime -P cmake_install.cmake
