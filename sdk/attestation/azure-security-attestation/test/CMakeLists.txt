# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-attestation-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)

add_executable (
     azure-attestation-test
     ut/macro_guard.cpp
     ut/attestation_test.cpp
     )

#create_map_file(azure-attestation-test azure-attestation-test.map)

target_link_libraries(azure-attestation-test PRIVATE Azure::azure-security-attestation gtest_main)

if (MSVC)
    target_compile_options(azure-attestation-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

gtest_discover_tests(azure-attestation-test TEST_PREFIX azure-security-attestation.)
