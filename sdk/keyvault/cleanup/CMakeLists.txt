# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (cleanup-keyvault LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  cleanup-keyvault
  cleanup.cpp
)
create_per_service_target_build_for_sample(keyvault cleanup-keyvault)

target_link_libraries(cleanup-keyvault PRIVATE azure-security-keyvault-secrets azure-security-keyvault-keys azure-security-keyvault-certificates azure-identity get-env-helper)
