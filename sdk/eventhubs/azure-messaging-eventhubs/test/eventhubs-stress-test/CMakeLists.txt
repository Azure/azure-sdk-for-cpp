# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

project(azure-messaging-eventhubs-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
  azure-messaging-eventhubs-stress-test
  eventhubs_stress_test.cpp
)

target_link_libraries(azure-messaging-eventhubs-stress-test PRIVATE azure-messaging-eventhubs azure-core azure-identity)

create_map_file(azure-messaging-eventhubs-stress-test azure-messaging-eventhubs-stress-test.map)
file(COPY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
