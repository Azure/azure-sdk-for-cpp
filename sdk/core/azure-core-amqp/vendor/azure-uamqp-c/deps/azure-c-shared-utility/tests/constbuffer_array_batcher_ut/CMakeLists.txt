#Copyright (c) Microsoft. All rights reserved.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName constbuffer_array_batcher_ut)

generate_cppunittest_wrapper(${theseTestsName})

set(${theseTestsName}_c_files
../../src/constbuffer_array_batcher.c
../real_test_files/real_constbuffer.c
../real_test_files/real_constbuffer_array.c
../real_test_files/real_memory_data.c
)

set(${theseTestsName}_h_files
../../inc/azure_c_shared_utility/constbuffer_array_batcher.h
../real_test_files/real_constbuffer.h
../real_test_files/real_constbuffer_renames.h
../real_test_files/real_constbuffer_array.h
../real_test_files/real_constbuffer_array_renames.h
../real_test_files/real_memory_data.h
../real_test_files/real_memory_data_renames.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
