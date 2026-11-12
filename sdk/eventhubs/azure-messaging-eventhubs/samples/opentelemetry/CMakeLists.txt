# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)
project(eventhubs-opentelemetry LANGUAGES CXX)

if(NOT TARGET Azure::azure-core-tracing-opentelemetry)
  message(FATAL_ERROR "The OpenTelemetry sample requires azure-core-tracing-opentelemetry (use a static vcpkg triplet on Windows).")
endif()

find_package(opentelemetry-cpp CONFIG REQUIRED)
add_executable(eventhubs-opentelemetry opentelemetry.cpp)
set_target_properties(eventhubs-opentelemetry PROPERTIES CXX_STANDARD 14 CXX_STANDARD_REQUIRED ON)
target_link_libraries(eventhubs-opentelemetry PRIVATE
  Azure::azure-messaging-eventhubs
  Azure::azure-identity
  Azure::azure-core-tracing-opentelemetry
  opentelemetry-cpp::otlp_http_exporter
  opentelemetry-cpp::otlp_http_log_record_exporter)
CREATE_PER_SERVICE_TARGET_BUILD_FOR_SAMPLE(eventhubs eventhubs-opentelemetry DISABLE_RUN)
