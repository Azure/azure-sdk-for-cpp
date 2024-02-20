# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

project(azure-messaging-eventhubs-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(WITH_STL ON)

find_package(opentelemetry-cpp CONFIG REQUIRED)
find_package(protobuf)
find_package(nlohmann_json)
find_package(CURL)

set(INCLUDE_FILES 
  inc/eventhubs_stress_scenarios.hpp
  scenarios/inc/event_sender.hpp
  scenarios/inc/opentelemetry_helpers.hpp
  scenarios/inc/batch_stress_tests.hpp
  )


set(SOURCE_FILES
  eventhubs_stress_test.cpp
  scenarios/src/opentelemetry_helpers.cpp
  scenarios/src/batch_stress_tests.cpp
  )


add_executable(
  azure-messaging-eventhubs-stress-test
  ${SOURCE_FILES} ${INCLUDE_FILES}
)

# Include the headers from the project.
target_include_directories(
  azure-messaging-eventhubs-stress-test
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/scenarios/inc>
)

target_link_libraries(azure-messaging-eventhubs-stress-test 
PRIVATE
azure-messaging-eventhubs
azure-identity
opentelemetry-cpp::ostream_span_exporter
opentelemetry-cpp::in_memory_span_exporter
opentelemetry-cpp::otlp_http_exporter
opentelemetry-cpp::otlp_http_log_record_exporter
opentelemetry-cpp::sdk
)

create_map_file(azure-messaging-eventhubs-stress-test azure-messaging-eventhubs-stress-test.map)
file(COPY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
