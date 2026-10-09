# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (cleanup-keyvault LANGUAGES CXX)
if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14 OR CMAKE_CXX_STANDARD STREQUAL "98")
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  cleanup-keyvault
  src/cleanup.cpp
)
create_per_service_target_build_for_sample(keyvault cleanup-keyvault)

target_link_libraries(cleanup-keyvault PRIVATE azure-security-keyvault-secrets azure-security-keyvault-keys azure-security-keyvault-certificates azure-identity get-env-helper)
