# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(local_server_sample)

add_executable(local_server_sample
    local_server_sample.cpp
)

target_compile_definitions(local_server_sample PRIVATE _azure_BUILDING_SAMPLES)

add_dependencies(local_server_sample Azure::azure-core-amqp)

target_link_libraries(local_server_sample PRIVATE Azure::azure-core-amqp)
