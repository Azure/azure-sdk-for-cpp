# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample1-basic-operations LANGUAGES CXX)

add_executable (
  sample1-basic-operations
  sample1_basic_operations.cpp
)
target_compile_features(sample1-basic-operations PRIVATE cxx_std_14)
set_target_properties(sample1-basic-operations PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(sample1-basic-operations PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample1-basic-operations)

target_link_libraries(sample1-basic-operations PRIVATE azure-security-keyvault-secrets azure-identity get-env-helper)
