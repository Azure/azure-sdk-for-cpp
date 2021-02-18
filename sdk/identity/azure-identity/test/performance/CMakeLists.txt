# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-identity-performance LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_IDENTITY_PERF_TEST_HEADER
  inc/azure/identity/test/performance/secret_credential.hpp
)

set(
  AZURE_IDENTITY_PERF_TEST_SOURCE
    src/main.cpp
)

# Name the binary to be created.
add_executable (
  azure-identity-performance
     ${AZURE_IDENTITY_PERF_TEST_HEADER} ${AZURE_IDENTITY_PERF_TEST_SOURCE}
)

# Include the headers from the project.
target_include_directories(
  azure-identity-performance
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-performance-stress` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-identity-performance PRIVATE azure-identity azure-performance-stress)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-identity-performance PROPERTIES FOLDER "Tests/Identity")
