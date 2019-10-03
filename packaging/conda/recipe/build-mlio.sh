#!/usr/bin/env bash

set -o errexit

if [[ $(uname -s) == Linux ]]; then
    _toolchain_file=$RECIPE_DIR/linux-toolchain.cmake
fi

# Build the dependencies first.
_dependencies_dir=$SRC_DIR/build/third-party

mkdir -p "$_dependencies_dir" && cd "$_"

cmake -GNinja\
      -DCMAKE_TOOLCHAIN_FILE="$_toolchain_file"\
      "$SRC_DIR/third-party"

for _dep in absl dlpack fmt gtest pybind11; do
    cmake --build . --target $_dep
done

# Now build libmlio.
mkdir -p "$SRC_DIR/build/mlio" && cd "$_"

cmake -GNinja\
      -DCMAKE_INSTALL_PREFIX="$PREFIX"\
      -DCMAKE_TOOLCHAIN_FILE="$_toolchain_file"\
      -DCMAKE_FIND_ROOT_PATH="$_dependencies_dir"\
      -DIconv_IS_BUILT_IN=FALSE\
      "$SRC_DIR"

cmake --build .
cmake --build . --target test

# Extract debug symbols; those will be installed with the debug
# package.
find lib -name 'libmlio*' -type f\
    -exec "$SRC_DIR/build-tools/strip-symbols" --extract '{}' ';'

# Install the runtime components.
cmake --install . --component runtime --verbose
