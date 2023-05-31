# Copyright(c) Microsoft Corporation.All rights reserved.
# SPDX - License - Identifier : MIT

cmake_minimum_required(VERSION 3.13)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(eventhub_sas_writer_sample)

add_executable(eventhub_sas_writer_sample
    eventhub_sas_writer_sample.cpp
)
add_dependencies(eventhub_sas_writer_sample Azure::azure-core-amqp)

target_link_libraries(eventhub_sas_writer_sample PRIVATE Azure::azure-core-amqp get-env-helper)
