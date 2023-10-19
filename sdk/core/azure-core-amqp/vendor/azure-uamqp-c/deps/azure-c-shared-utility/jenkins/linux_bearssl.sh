#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.

set -e

# Print version
cat /etc/*release | grep VERSION*
gcc --version
curl --version

# Set the default cores
CORES=$(grep -c ^processor /proc/cpuinfo 2>/dev/null || sysctl -n hw.ncpu)
cmake . -Bcmake -Duse_bearssl:BOOL=ON -Drun_unittests:BOOL=ON -Duse_openssl:BOOL=OFF -D CMAKE_C_COMPILER=gcc
cd cmake

make --jobs=$CORES

ctest -j $CORES --output-on-failure
