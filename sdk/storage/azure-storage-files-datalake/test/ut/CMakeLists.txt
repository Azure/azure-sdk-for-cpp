# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-storage-files-datalake-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable (
  azure-storage-files-datalake-test
    datalake_directory_client_test.cpp
    datalake_directory_client_test.hpp
    datalake_file_client_test.cpp
    datalake_file_client_test.hpp
    datalake_file_system_client_test.cpp
    datalake_file_system_client_test.hpp
    datalake_path_client_test.cpp
    datalake_path_client_test.hpp
    datalake_sas_test.cpp
    datalake_service_client_test.cpp
    datalake_service_client_test.hpp
    macro_guard.cpp
    # Include shared test source code
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common/test/ut/test_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common/test/ut/test_base.hpp
)
create_per_service_target_build(storage azure-storage-files-datalake-test)
create_map_file(azure-storage-files-datalake-test azure-storage-files-datalake-test.map)

# Include shared test headers
target_include_directories(azure-storage-files-datalake-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common)

target_link_libraries(azure-storage-files-datalake-test PRIVATE azure-storage-files-datalake azure-identity azure-core-test-fw gtest gtest_main gmock)

# gtest_discover_tests will scan the test from azure-storage-files-datalake-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-storage-files-datalake-test
     TEST_PREFIX azure-storage-files-datalake.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES)
