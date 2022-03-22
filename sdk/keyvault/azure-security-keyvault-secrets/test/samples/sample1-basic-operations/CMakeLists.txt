# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample1-basic-operations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample1-basic-operations
  sample1_basic_operations.cpp
)
create_per_service_target_build_for_sample(keyvault sample1-basic-operations)

target_link_libraries(sample1-basic-operations PRIVATE azure-security-keyvault-secrets azure-identity get-env-helper)
