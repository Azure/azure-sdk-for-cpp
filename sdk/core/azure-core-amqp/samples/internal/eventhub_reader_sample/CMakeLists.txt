# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(eventhub_reader_sample)

# Samples
add_executable(eventhub_reader_sample
    eventhub_reader_sample.cpp
)

target_compile_definitions(eventhub_reader_sample PRIVATE _azure_BUILDING_SAMPLES)

add_dependencies(eventhub_reader_sample Azure::azure-core-amqp)

# Link test executable against the prototype.
target_link_libraries(eventhub_reader_sample PRIVATE Azure::azure-core-amqp)
