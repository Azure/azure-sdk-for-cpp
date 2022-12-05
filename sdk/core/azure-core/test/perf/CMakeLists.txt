# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-core-perf LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_CORE_PERF_TEST_HEADER
  inc/azure/core/test/nullable_test.hpp
)

set(
  AZURE_CORE_PERF_TEST_SOURCE
  src/azure_core_perf_test.cpp
)

# Name the binary to be created.
add_executable (
  azure-core-perf
     ${AZURE_CORE_PERF_TEST_HEADER} ${AZURE_CORE_PERF_TEST_SOURCE}
)

# Include the headers from the project.
target_include_directories(
  azure-core-perf
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-core-perf PRIVATE azure-core azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-core-perf PROPERTIES FOLDER "Tests/Core")