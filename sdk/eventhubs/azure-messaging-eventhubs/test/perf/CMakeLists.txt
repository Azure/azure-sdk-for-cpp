# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-messaging-eventhubs-perf LANGUAGES CXX)

set(
  AZURE_EVENTHUBS_PERF_TEST_HEADER
  inc/azure/messaging/eventhubs/test/eventhubs_batch_perf_test.hpp
)

set(
  AZURE_EVENTHUBS_PERF_TEST_SOURCE
    src/azure_eventhubs_perf_test.cpp
)

# Name the binary to be created.
add_executable (
  azure-messaging-eventhubs-perf
     ${AZURE_EVENTHUBS_PERF_TEST_HEADER} ${AZURE_EVENTHUBS_PERF_TEST_SOURCE}
)
target_compile_features(azure-messaging-eventhubs-perf PRIVATE cxx_std_14)
set_target_properties(azure-messaging-eventhubs-perf PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(azure-messaging-eventhubs-perf PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(eventhubs azure-messaging-eventhubs-perf)
create_map_file(azure-messaging-eventhubs-perf azure-messaging-eventhubs-perf.map)

# Include the headers from the project.
target_include_directories(
  azure-messaging-eventhubs-perf
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-messaging-eventhubs-perf PRIVATE azure-identity azure-messaging-eventhubs azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-messaging-eventhubs-perf PROPERTIES FOLDER "Tests/eventhubs")
