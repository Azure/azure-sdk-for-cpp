# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample3-get-keys LANGUAGES CXX)

add_executable (
  sample3-get-keys
  sample3_get_keys.cpp
)
target_compile_features(sample3-get-keys PRIVATE cxx_std_14)
set_target_properties(sample3-get-keys PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(sample3-get-keys PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample3-get-keys)

target_link_libraries(sample3-get-keys PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
