# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(eventhub_writer_async_sample)

add_executable(eventhub_writer_async_sample
    eventhub_async_writer_sample.cpp
)

target_compile_definitions(eventhub_writer_async_sample PRIVATE _azure_BUILDING_SAMPLES)

add_dependencies(eventhub_writer_async_sample Azure::azure-core-amqp)

# Link test executable against the prototype.
target_link_libraries(eventhub_writer_async_sample PRIVATE Azure::azure-core-amqp get-env-helper)
