# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(local_client_async_sample)

add_executable(local_client_async_sample
    local_client_async_sample.cpp
)
add_dependencies(local_client_async_sample Azure::azure-core-amqp)

target_include_directories(local_client_async_sample PRIVATE ${azure-core-amqp_SOURCE_DIR})

target_link_libraries(local_client_async_sample PRIVATE Azure::azure-core-amqp)
