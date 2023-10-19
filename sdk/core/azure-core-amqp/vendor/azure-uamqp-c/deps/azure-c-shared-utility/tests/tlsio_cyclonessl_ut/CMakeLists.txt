#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName tlsio_cyclonessl_ut)

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

set(${theseTestsName}_test_files
	${theseTestsName}.c
)

set(${theseTestsName}_c_files
	../../src/tlsio_cyclonessl.c
	../real_test_files/real_crt_abstractions.c
)

#CycloneSSL has a pretty big list of warnings which need to be ignored
set_source_files_properties(../../src/tlsio_cyclonessl.c PROPERTIES COMPILE_FLAGS "/wd4200 /wd4201 /wd4244 /wd4100 /wd4267 /wd4701 /wd4703 /wd4389 /wd4005 /wd4996")

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
