# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-storage-queues-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")

add_executable (
  azure-storage-queues-test
    macro_guard.cpp
    queue_client_messages_test.cpp
    queue_client_test.cpp
    queue_client_test.hpp
    queue_sas_test.cpp
    queue_service_client_test.cpp
    # Include shared test source code
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common/test/ut/test_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common/test/ut/test_base.hpp
)
create_per_service_target_build(storage azure-storage-queues-test)
create_map_file(azure-storage-queues-test azure-storage-queues-test.map)

# Include shared test headers
target_include_directories(azure-storage-queues-test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../azure-storage-common)

target_link_libraries(azure-storage-queues-test PRIVATE azure-storage-queues azure-storage-blobs azure-identity azure-core-test-fw gtest gtest_main gmock)

# gtest_discover_tests will scan the test from azure-storage-queues-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-storage-queues-test
     TEST_PREFIX azure-storage-queues.
     NO_PRETTY_TYPES
     NO_PRETTY_VALUES)
