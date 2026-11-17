# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(basic-operations)
add_subdirectory(produce-events)
add_subdirectory(consume-events)

option(AZURE_EVENTHUBS_BUILD_OPENTELEMETRY_SAMPLE "Build the opt-in OTLP/HTTP exporter sample" OFF)
if(AZURE_EVENTHUBS_BUILD_OPENTELEMETRY_SAMPLE)
  add_subdirectory(opentelemetry)
endif()
