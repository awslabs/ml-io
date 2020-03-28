#!/usr/bin/env bash

set -o errexit

# Build the Python C extensions.
cd "$SRC_DIR/build/mlio"

cmake -DMLIO_INCLUDE_LIB=OFF\
      -DMLIO_INCLUDE_CONTRIB=ON\
      -DMLIO_INCLUDE_PYTHON_EXTENSION=ON\
      -DMLIO_INCLUDE_ARROW_INTEGRATION=ON\
      -DPYTHON_EXECUTABLE="$(command -v python)"\
      "$SRC_DIR"

cmake --build . --target clean
cmake --build . --target mlio-py
cmake --build . --target mlio-arrow
cmake --build . --target mlio-contrib

# Using the python-config script is not reliable as Conda does not
# include it in the dependency closure.
_ext_suffix=$(python -c 'import sysconfig; print(sysconfig.get_config_var("EXT_SUFFIX"))')

# Strip the extension modules.
find "$SRC_DIR/src/mlio-py" -name \*$_ext_suffix -type f\
    -exec "$SRC_DIR/build-tools/strip-symbols" '{}' ';'

# Install the wheel.
python -m pip install "$SRC_DIR/src/mlio-py"\
    --no-deps --ignore-installed --no-cache-dir -vvv
