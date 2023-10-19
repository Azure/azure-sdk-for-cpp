#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName refcount_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
	some_refcount_impl.c
)

set(${theseTestsName}_h_files
	some_refcount_impl.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")
