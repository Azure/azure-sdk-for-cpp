#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName tlsio_bearssl_ut)

set(BEARSSL_ROOT_DIR $ENV{BEARSSL_ROOT_DIR} CACHE PATH "")
#set(BEARSSL_ROOT_DIR "C:/Users/markrad/S/BearSSL")
include_directories("${BEARSSL_ROOT_DIR}/include")

set(${theseTestsName}_test_files
    ${theseTestsName}.c
)

set(${theseTestsName}_c_files
    ../../adapters/tlsio_bearssl.c
	../../src/vector.c
	../../src/buffer.c
)

set(${theseTestsName}_h_files
)

add_definitions(-DUSE_BEAR_SSL)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
