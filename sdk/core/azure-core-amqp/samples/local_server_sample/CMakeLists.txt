# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(local_server_sample)

# Unit Tests
add_executable(local_server_sample
    local_server_sample.cpp
)
add_dependencies(local_server_sample azure-core-amqp)

target_include_directories(local_server_sample PRIVATE ${azure-core-amqp_SOURCE_DIR})

# Link test executable against gtest & gtest_main
target_link_libraries(local_server_sample PRIVATE azure-core-amqp)
