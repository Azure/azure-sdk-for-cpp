#Copyright (c) Microsoft. All rights reserved.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName srw_lock_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
srw_lock_mocked.c
../../adapters/string_utils.c
)

set(${theseTestsName}_cpp_files
)

set(${theseTestsName}_h_files
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

