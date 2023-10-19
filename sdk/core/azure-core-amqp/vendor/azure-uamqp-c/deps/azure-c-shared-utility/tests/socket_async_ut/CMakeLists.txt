#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName socket_async_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
../../pal/socket_async.c
)

include_directories(.)
include_directories(../../pal/inc)

# Windows will eventually be built with a socket_async_win32.c because
# the function signatures don't match Linux. That file will need
# different unit tests because of the differing socket semantics


if(WIN32)
	include_directories(./win32_fake_linux)
else()
	include_directories(../../pal/linux)
endif()

set(${theseTestsName}_h_files
test_defines.h
keep_alive.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C11)