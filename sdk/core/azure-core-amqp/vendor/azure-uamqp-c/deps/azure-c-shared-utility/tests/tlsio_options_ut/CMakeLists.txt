#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName tlsio_options_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
	../../pal/tlsio_options.c
	../../src/optionhandler.c
	../../src/crt_abstractions.c
	../../src/vector.c
)

set(${theseTestsName}_h_files
    gballoc_ut_impl_1.h
    gballoc_ut_impl_2.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

