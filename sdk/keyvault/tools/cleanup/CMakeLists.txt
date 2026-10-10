# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (cleanup-keyvault LANGUAGES CXX)

add_executable (
  cleanup-keyvault
  src/cleanup.cpp
)
target_compile_features(cleanup-keyvault PRIVATE cxx_std_14)
set_target_properties(cleanup-keyvault PROPERTIES CXX_STANDARD_REQUIRED ON)
create_per_service_target_build_for_sample(keyvault cleanup-keyvault)

target_link_libraries(cleanup-keyvault PRIVATE azure-security-keyvault-secrets azure-security-keyvault-keys azure-security-keyvault-certificates azure-identity get-env-helper)
