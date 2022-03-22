# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-identity-perf LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_IDENTITY_PERF_TEST_HEADER
  inc/azure/identity/test/secret_credential_test.hpp
)

set(
  AZURE_IDENTITY_PERF_TEST_SOURCE
    src/azure_identity_perf_test.cpp
)

# Name the binary to be created.
add_executable (
  azure-identity-perf
     ${AZURE_IDENTITY_PERF_TEST_HEADER} ${AZURE_IDENTITY_PERF_TEST_SOURCE}
)
create_per_service_target_build(identity azure-identity-perf)

create_map_file(azure-identity-perf azure-identity-perf.map)

# Include the headers from the project.
target_include_directories(
  azure-identity-perf
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-identity-perf PRIVATE azure-identity azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-identity-perf PROPERTIES FOLDER "Tests/Identity")
