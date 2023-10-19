#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

if(NOT ${use_http})
	message(FATAL_ERROR "httpapiex_ut being generated without HTTP support")
endif()

set(theseTestsName httpapiex_ut )

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
../../src/httpapiex.c
../real_test_files/real_vector.c
../real_test_files/real_crt_abstractions.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
