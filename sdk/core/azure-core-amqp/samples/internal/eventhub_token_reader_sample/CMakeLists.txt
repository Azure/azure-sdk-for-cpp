# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(eventhub_token_reader_sample)

add_executable(eventhub_token_reader_sample
    eventhub_token_reader_sample.cpp
)
add_dependencies(eventhub_token_reader_sample azure-core-amqp)

target_link_libraries(eventhub_token_reader_sample PRIVATE azure-core-amqp Azure::azure-identity get-env-helper)
