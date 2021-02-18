# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-core-performance LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_CORE_PERF_TEST_HEADER
  inc/azure/core/test/performance/nullable.hpp
)

set(
  AZURE_CORE_PERF_TEST_SOURCE
  src/main.cpp
)

# Name the binary to be created.
add_executable (
  azure-core-performance
     ${AZURE_CORE_PERF_TEST_HEADER} ${AZURE_CORE_PERF_TEST_SOURCE}
)

# Include the headers from the project.
target_include_directories(
  azure-core-performance
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-performance-stress` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-core-performance PRIVATE azure-core azure-performance-stress)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-core-performance PROPERTIES FOLDER "Tests/Core")
