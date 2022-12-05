# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-storage-common-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable (
  azure-storage-common-test
    bearer_token_test.cpp
    crypt_functions_test.cpp
    metadata_test.cpp
    storage_credential_test.cpp
    test_base.cpp
    test_base.hpp
)
create_per_service_target_build(storage azure-storage-common-test)
create_map_file(azure-storage-common-test azure-storage-common-test.map)

if (MSVC)
  target_compile_options(azure-storage-common-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_include_directories(azure-storage-common-test PRIVATE test)

# azure-storage-common-test depends on azure-storage-common and azure-storage-blobs
# blobs is required as a client to test the credentials in common
# only the test binary requires the common and blobs libs
target_link_libraries(azure-storage-common-test PRIVATE azure-storage-common azure-storage-blobs azure-identity azure-core-test-fw gtest gtest_main gmock)

# gtest_discover_tests will scan the test from azure-storage-common-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-storage-common-test
     TEST_PREFIX azure-storage-common.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES
     DISCOVERY_TIMEOUT 600)