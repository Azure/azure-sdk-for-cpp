#Copyright (c) Microsoft. All rights reserved.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName constbuffer_array_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
    ../../src/constbuffer_array.c
)

set(${theseTestsName}_h_files
    ../../inc/azure_c_shared_utility/constbuffer_array.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
