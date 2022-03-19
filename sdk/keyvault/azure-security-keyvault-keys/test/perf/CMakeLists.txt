# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-keys-perf LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_KEYVAULT_KEY_PERF_TEST_HEADER
  inc/azure/keyvault/keys/test/get_key_test.hpp
)

set(
  AZURE_KEYVAULT_KEY_PERF_TEST_SOURCE
    src/azure_security_keyvault_keys_perf_test.cpp
)

# Name the binary to be created.
add_executable (
  azure-security-keyvault-keys-perf
     ${AZURE_KEYVAULT_KEY_PERF_TEST_HEADER} ${AZURE_KEYVAULT_KEY_PERF_TEST_SOURCE}
)
create_per_service_target_build(azure-security-keyvault-keys-perf keyvault false)
create_map_file(azure-security-keyvault-keys-perf azure-security-keyvault-keys-perf.map)

# Include the headers from the project.
target_include_directories(
  azure-security-keyvault-keys-perf
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-security-keyvault-keys-perf PRIVATE azure-identity azure-security-keyvault-keys azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-security-keyvault-keys-perf PROPERTIES FOLDER "Tests/Keyvault")
