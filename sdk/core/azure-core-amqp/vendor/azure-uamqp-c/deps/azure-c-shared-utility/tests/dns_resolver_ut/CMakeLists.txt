#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName dns_resolver_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
../../src/dns_resolver_sync.c
../../src/crt_abstractions.c
)

set(${theseTestsName}_h_files
)

include_directories(.)
include_directories(../../inc/azure_c_shared_utility)

if(WIN32)
	include_directories(./win32_fake_linux)
else()
	include_directories(../../pal/linux)
endif()

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
