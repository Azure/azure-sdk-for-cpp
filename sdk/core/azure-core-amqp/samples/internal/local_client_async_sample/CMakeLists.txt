# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(local_client_async_sample)

add_executable(local_client_async_sample
    local_client_async_sample.cpp
)

target_compile_definitions(local_client_async_sample PRIVATE _azure_BUILDING_SAMPLES)

add_dependencies(local_client_async_sample Azure::azure-core-amqp)

target_link_libraries(local_client_async_sample PRIVATE Azure::azure-core-amqp)
