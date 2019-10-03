#!/usr/bin/env bash

set -o errexit

if [[ $(uname -s) == Darwin ]]; then
    _dbg_suffix=dSYM
else
    _dbg_suffix=debug
fi

mkdir -p "$PREFIX/lib"

# Copy debug symbols to the output directory.
cp -a "$SRC_DIR/build/mlio/lib/"libmlio*.$_dbg_suffix "$PREFIX/lib/"
