# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

project(eventhub_get_eventhub_properties_sample)

add_executable(eventhub_get_eventhub_properties_sample
    eventhub_get_eventhub_properties_sample.cpp
)

target_compile_definitions(eventhub_get_eventhub_properties_sample PRIVATE _azure_BUILDING_SAMPLES)

add_dependencies(eventhub_get_eventhub_properties_sample Azure::azure-core-amqp)

target_link_libraries(eventhub_get_eventhub_properties_sample PRIVATE Azure::azure-core-amqp Azure::azure-identity get-env-helper)
