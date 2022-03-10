# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-attestation-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

include(GoogleTest)

add_executable (
     azure-security-attestation-test
     macro_guard.cpp
     crypto_test.cpp
     attestation_test.cpp
     token_test.cpp 
     policygetset_test.cpp
     attestation_collateral.cpp 
     attestation_metadata.cpp)

create_per_service_target_build(azure-security-attestation-test attestation)

create_map_file(azure-security-attestation-test azure-security-attestation-test.map)

target_link_libraries(azure-security-attestation-test PRIVATE Azure::azure-security-attestation azure-identity azure-core-test-fw gtest_main)

if (MSVC)
    target_compile_options(azure-security-attestation-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

gtest_discover_tests(azure-security-attestation-test TEST_PREFIX azure-security-attestation.)
