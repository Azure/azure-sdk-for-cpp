# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

set(azure-storage-blobs-test-fault-injector)

project (azure-storage-blobs-test-fault-injector LANGUAGES CXX)
if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14 OR CMAKE_CXX_STANDARD STREQUAL "98")
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  azure-storage-blobs-test-fault-injector
    azure_storage_blobs_fault_injector_test.cpp
)

target_compile_definitions(azure-storage-blobs-test-fault-injector PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build(storage azure-storage-blobs-test-fault-injector)

target_link_libraries(azure-storage-blobs-test-fault-injector PRIVATE azure-storage-blobs)
