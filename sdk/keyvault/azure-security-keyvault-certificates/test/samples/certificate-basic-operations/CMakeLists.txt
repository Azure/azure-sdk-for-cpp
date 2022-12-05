# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (certificate-basic-operations LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  certificate-basic-operations
  certificate_basic_operations.cpp
)

create_per_service_target_build_for_sample(keyvault certificate-basic-operations)

target_link_libraries(certificate-basic-operations PRIVATE azure-security-keyvault-certificates azure-identity get-env-helper)