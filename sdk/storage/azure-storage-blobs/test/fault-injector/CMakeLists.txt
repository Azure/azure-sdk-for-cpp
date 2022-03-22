# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

set(azure-storage-blobs-test-fault-injector)

project (azure-storage-blobs-test-fault-injector LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  azure-storage-blobs-test-fault-injector
    azure_storage_blobs_fault_injector_test.cpp
)
create_per_service_target_build(storage azure-storage-blobs-test-fault-injector)

target_link_libraries(azure-storage-blobs-test-fault-injector PRIVATE azure-storage-blobs)
