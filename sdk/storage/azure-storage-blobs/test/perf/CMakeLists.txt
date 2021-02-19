# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-storage-blobs-perf LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_STORAGE_BLOBS_PERF_TEST_HEADER
  inc/azure/storage/blobs/test/perf/download_blob.hpp
)

set(
  AZURE_STORAGE_BLOBS_PERF_TEST_SOURCE
    src/main.cpp
)

# Name the binary to be created.
add_executable (
  azure-storage-blobs-perf
     ${AZURE_STORAGE_BLOBS_PERF_TEST_HEADER} ${AZURE_STORAGE_BLOBS_PERF_TEST_SOURCE}
)

# Include the headers from the project.
target_include_directories(
  azure-storage-blobs-perf
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-storage-blobs-perf PRIVATE azure-storage-blobs azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-storage-blobs-perf PROPERTIES FOLDER "Tests/Storage")
