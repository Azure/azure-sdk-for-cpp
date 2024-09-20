#!/bin/bash
# Copyright (c) Microsoft. All rights reserved. Licensed under the MIT 
# license. See LICENSE file in the project root for full license 
# information.

# Tested on RPi2 debian verion 7.8

install_root="/home/jenkins" 
build_root=$(cd "$(dirname "$0")/.." && pwd) 
cd $build_root

# ----------------------------------------------------------------------------- 
# -- helper subroutines 
# -----------------------------------------------------------------------------
checkExists() {
    if hash $1 2>/dev/null;
    then
        return 1
    else
        echo "$1" not found. Please make sure that "$1" is installed and available in the path.
        exit 1
    fi
}
# ----------------------------------------------------------------------------- 
# -- Check for environment pre-requisites. This script requires -- that 
# the following programs work: -- curl build-essential(g++,gcc,make) 
# cmake git 
# -----------------------------------------------------------------------------
checkExists g++ 
checkExists gcc 
checkExists make 
checkExists cmake 
checkExists git

# ----------------------------------------------------------------------------- 
# -- Check for RPiTools directory.
# -----------------------------------------------------------------------------
if [ ! -d "$install_root/RPiTools" ];
then
   echo ---------- Raspberry Pi tool-chain absent ----------
   exit 1
fi

# ----------------------------------------------------------------------------- 
# -- Set environment variable
# -----------------------------------------------------------------------------
cd $install_root/RPiTools/tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian-x64/arm-linux-gnueabihf 
export RPI_ROOT=$(pwd)

mkdir $build_root/cmake
cd $build_root/cmake

# ----------------------------------------------------------------------------- 
# -- Create toolchain-rpi.cmake 
# -----------------------------------------------------------------------------
echo ---------- Creating toolchain cmake file ---------- 
FILE="$build_root/cmake/toolchain-rpi.cmake" 

/bin/cat <<EOM >$FILE
INCLUDE(CMakeForceCompiler) 

SET(CMAKE_SYSTEM_NAME Linux) # this one is important 
SET(CMAKE_SYSTEM_VERSION 1) # this one not so much

# this is the location of the amd64 toolchain targeting the Raspberry Pi
SET(CMAKE_C_COMPILER ${RPI_ROOT}/../bin/arm-linux-gnueabihf-gcc)

# this is the file system root of the target
SET(CMAKE_FIND_ROOT_PATH ${RPI_ROOT})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)

# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY) 
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY) 
EOM

# ----------------------------------------------------------------------------- 
# -- Build uAMQP
# -----------------------------------------------------------------------------
echo ---------- Building the SDK by executing build.sh script ---------- 
cmake .. -DCMAKE_TOOLCHAIN_FILE=toolchain-rpi.cmake -DcompileOption_C:STRING="--sysroot=$RPI_ROOT"
make
[ $? -eq 0 ] || exit $?
