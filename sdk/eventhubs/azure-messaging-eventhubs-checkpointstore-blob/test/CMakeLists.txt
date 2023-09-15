# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-messaging-eventhubs-blobcheckpointstore-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")
include(TestProxyPrep)
SetUpTestProxy("sdk/eventhubs")

################## Unit Tests ##########################
add_executable (
  azure-messaging-eventhubs-blobcheckpointstore-test
    blob_checkpoint_store_test.cpp
)

create_per_service_target_build(eventhubs azure-messaging-eventhubs-blobcheckpointstore-test)
create_map_file(azure-messaging-eventhubs-blobcheckpointstore-test azure-messaging-eventhubs-blobcheckpointstore-test.map)

if (MSVC)
  target_compile_options(azure-messaging-eventhubs-blobcheckpointstore-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(
    azure-messaging-eventhubs-blobcheckpointstore-test
      PRIVATE 
        azure-messaging-eventhubs-checkpoint-blob
        azure-messaging-eventhubs 
        azure-core-test-fw
        azure-identity 
        gtest 
        gtest_main 
        gmock
)

# Adding private headers so we can test the private APIs with no relative paths include.
target_include_directories (
    azure-messaging-eventhubs-blobcheckpointstore-test 
      PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)

# gtest_add_tests will scan the test from azure-messaging-eventhubs-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-messaging-eventhubs-blobcheckpointstore-test
  TEST_PREFIX azure-messaging-eventhubs.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
  DISCOVERY_TIMEOUT 600)
