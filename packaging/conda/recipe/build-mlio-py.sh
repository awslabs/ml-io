#!/usr/bin/env bash

set -o errexit

# Build the Python C extensions.
cd "$SRC_DIR/build/conda"

cmake -DPYTHON_EXECUTABLE="$PYTHON"\
      -DMLIO_INCLUDE_LIB=FALSE\
      -DMLIO_INCLUDE_PYTHON_EXTENSION=TRUE\
      "$SRC_DIR"

cmake --build . --target mlio-py-core
cmake --build . --target mlio-py-arrow

# Install the wheel.
cd "$SRC_DIR/src/mlio-py"

"$PYTHON" -m pip install . --no-deps --ignore-installed --no-cache-dir -vvv
