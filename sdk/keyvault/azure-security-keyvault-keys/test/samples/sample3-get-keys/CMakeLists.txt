# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample3-get-keys LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample3-get-keys
  sample3_get_keys.cpp
)
create_per_service_target_build(sample3-get-keys keyvault)

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
file(APPEND ${CMAKE_BINARY_DIR}/keyvault-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/sample3-get-keys\n")

target_link_libraries(sample3-get-keys PRIVATE azure-security-keyvault-keys azure-identity)
