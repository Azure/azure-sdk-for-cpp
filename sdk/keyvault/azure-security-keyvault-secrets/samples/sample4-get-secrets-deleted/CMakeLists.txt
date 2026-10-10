# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample4-get-secrets-deleted LANGUAGES CXX)

add_executable (
  sample4-get-secrets-deleted
  sample4_get_secrets_deleted.cpp
)
target_compile_features(sample4-get-secrets-deleted PRIVATE cxx_std_14)
set_target_properties(sample4-get-secrets-deleted PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(sample4-get-secrets-deleted PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample4-get-secrets-deleted)

target_link_libraries(sample4-get-secrets-deleted PRIVATE azure-security-keyvault-secrets azure-identity get-env-helper)
