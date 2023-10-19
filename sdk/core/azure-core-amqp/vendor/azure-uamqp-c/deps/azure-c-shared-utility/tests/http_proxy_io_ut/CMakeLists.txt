#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName http_proxy_io_ut)

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
	../../src/http_proxy_io.c
	../real_test_files/real_crt_abstractions.c
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
