#!/bin/bash
# Copyright (c) Microsoft. All rights reserved.
# Licensed under the MIT license. See LICENSE file in the project root for full license information.
#

set -e

script_dir=$(cd "$(dirname "$0")" && pwd)
build_root=$(cd "${script_dir}/.." && pwd)
log_dir=$build_root
make_install=
build_folder=$build_root"/cmake/uhttp"

rm -r -f $build_folder
mkdir -p $build_folder
pushd $build_folder
cmake ../.. -Drun_valgrind:BOOL=ON -Drun_unittests:bool=ON
if [ $? != 0 ];then
echo "Failure running cmake"
fi

cmake --build . -- --jobs=$(nproc)
if [ $? != 0 ];then
echo "Failure running build"
fi

ctest -j $(nproc) -C "debug" -V
if [ $? != 0 ];then
echo "Failure running ctest"
fi

popd
: