#Copyright (c) Microsoft. All rights reserved.
#Licensed under the MIT license. See LICENSE file in the project root for full license information.

cmake_minimum_required (VERSION 3.5)

set(theseTestsName connectionstringparser_ut)

generate_cppunittest_wrapper(${theseTestsName})

include_directories(${SHARED_UTIL_REAL_TEST_FOLDER})

set(${theseTestsName}_c_files
    ../real_test_files/real_map.c
    ../real_test_files/real_string_tokenizer.c
    ../real_test_files/real_strings.c
    ${SHARED_UTIL_SRC_FOLDER}/crt_abstractions.c
    ${SHARED_UTIL_SRC_FOLDER}/connection_string_parser.c
)

set(${theseTestsName}_h_files
    ../real_test_files/real_map.h
    ../real_test_files/real_string_tokenizer.h
    ../real_test_files/real_strings.h
)

build_c_test_artifacts(${theseTestsName} ON "tests/azure_c_shared_utility_tests")

compile_c_test_artifacts_as(${theseTestsName} C99)
