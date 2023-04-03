# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

cmake_minimum_required(VERSION 3.15)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(eventhub_writer_async_sample)

add_executable(eventhub_writer_async_sample
    eventhub_async_writer_sample.cpp
)
add_dependencies(eventhub_writer_async_sample azure-core-amqp)

target_include_directories(eventhub_writer_async_sample PRIVATE ${azure-core-amqp_SOURCE_DIR})

# Link test executable against the prototype.
target_link_libraries(eventhub_writer_async_sample PRIVATE azure-core-amqp)
