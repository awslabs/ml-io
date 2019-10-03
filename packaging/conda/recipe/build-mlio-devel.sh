#!/usr/bin/env bash

set -o errexit

cd "$SRC_DIR/build/conda"

# Install the development components.
cmake -DCOMPONENT=devel -P cmake_install.cmake
