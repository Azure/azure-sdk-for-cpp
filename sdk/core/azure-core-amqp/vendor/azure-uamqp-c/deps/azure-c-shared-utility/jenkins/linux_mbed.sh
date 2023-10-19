#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

set -e

build_root=$(cd "$(dirname "$0")/.." && pwd)
cd $build_root

build_folder=$build_root"/cmake"

# Set the default cores
CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)

rm -r -f $build_folder
mkdir -p $build_folder
pushd $build_folder
cmake -Drun_valgrind:BOOL=ON -Duse_mbedtls=ON -Duse_openssl=OFF $build_root -Drun_unittests:BOOL=ON -D CMAKE_C_COMPILER=gcc -D CMAKE_CXX_COMPILER=g++
make --jobs=$CORES

ctest -j $CORES --output-on-failure

popd
