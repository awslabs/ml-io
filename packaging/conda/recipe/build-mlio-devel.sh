#!/usr/bin/env bash

set -o errexit

# Install the development components.
cmake --install "$SRC_DIR/build/mlio" --component devel --verbose
