# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(local_client_async_sample)

# Unit Tests
add_executable(local_client_async_sample
    local_client_async_sample.cpp
)
add_dependencies(local_client_async_sample azure-core-amqp)

target_include_directories(local_client_async_sample PRIVATE ${azure-core-amqp_SOURCE_DIR})

# Link test executable against the prototype.
target_link_libraries(local_client_async_sample PRIVATE azure-core-amqp)
