# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

project(eventhub_token_reader_sample)

add_executable(eventhub_token_reader_sample
    eventhub_token_reader_sample.cpp
)

target_compile_definitions(eventhub_token_reader_sample PRIVATE _azure_BUILDING_SAMPLES)

add_dependencies(eventhub_token_reader_sample azure-core-amqp)

target_link_libraries(eventhub_token_reader_sample PRIVATE azure-core-amqp Azure::azure-identity get-env-helper)
