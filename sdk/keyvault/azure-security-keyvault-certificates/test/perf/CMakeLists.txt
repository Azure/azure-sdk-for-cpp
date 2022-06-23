# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-certificates-perf LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_KEYVAULT_CERTIFICATES_PERF_TEST_HEADER
  inc/azure/keyvault/certificates/test/get_certificate_test.hpp
)

set(
  AZURE_KEYVAULT_CERTIFICATES_PERF_TEST_SOURCE
    src/azure_security_keyvault_certificates_perf_test.cpp
)

# Name the binary to be created.
add_executable (
  azure-security-keyvault-certificates-perf
     ${AZURE_KEYVAULT_CERTIFICATES_PERF_TEST_HEADER} ${AZURE_KEYVAULT_CERTIFICATES_PERF_TEST_SOURCE}
)
create_per_service_target_build(keyvault azure-security-keyvault-certificates-perf)
create_map_file(azure-security-keyvault-certificates-perf azure-security-keyvault-certificates-perf.map)

# Include the headers from the project.
target_include_directories(
  azure-security-keyvault-certificates-perf
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-security-keyvault-certificates-perf PRIVATE azure-identity azure-security-keyvault-certificates azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-security-keyvault-certificates-perf PROPERTIES FOLDER "Tests/Keyvault")
