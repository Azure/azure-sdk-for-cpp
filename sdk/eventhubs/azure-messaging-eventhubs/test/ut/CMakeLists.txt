# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (azure-messaging-eventhubs-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")
include(TestProxyPrep)
SetUpTestProxy("sdk/eventhubs")

################## Unit Tests ##########################
add_executable (
  azure-messaging-eventhubs-test
    checkpoint_store_test.cpp
    consumer_client_test.cpp
    event_data_test.cpp
    eventhubs_admin_client_test.cpp
    eventhubs_admin_client.cpp
    eventhubs_admin_client.hpp
    eventhubs_test_base.hpp
    processor_load_balancer_test.cpp
    processor_test.cpp
    producer_client_test.cpp
    retry_operation_test.cpp
    round_trip_test.cpp
    test_checkpoint_store.hpp
)

target_compile_definitions(azure-messaging-eventhubs-test PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(eventhubs azure-messaging-eventhubs-test)
create_map_file(azure-messaging-eventhubs-test azure-messaging-eventhubs-test.map)

if (MSVC)
  target_compile_options(azure-messaging-eventhubs-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(
    azure-messaging-eventhubs-test
      PRIVATE 
        azure-messaging-eventhubs 
        azure-core-test-fw
        azure-identity 
        gtest 
        gtest_main 
        gmock
)

# Adding private headers so we can test the private APIs with no relative paths include.
target_include_directories (
    azure-messaging-eventhubs-test 
      PRIVATE 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)
cmake_policy(PUSH)
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.31.0)
cmake_policy(SET CMP0178 OLD)
endif()
# gtest_add_tests will scan the test from azure-messaging-eventhubs-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-messaging-eventhubs-test
  TEST_PREFIX azure-messaging-eventhubs.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
  DISCOVERY_TIMEOUT 600)
cmake_policy(POP)
