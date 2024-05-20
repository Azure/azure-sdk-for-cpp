# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.18)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(local_client_sample)

add_executable(local_client_sample
    local_client_sample.cpp
)
add_dependencies(local_client_sample Azure::azure-core-amqp)

target_link_libraries(local_client_sample PRIVATE Azure::azure-core-amqp)
