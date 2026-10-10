# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample5-sign-verify LANGUAGES CXX)

add_executable (
  sample5-sign-verify
  sample5_sign_verify.cpp
)
target_compile_features(sample5-sign-verify PRIVATE cxx_std_14)
set_target_properties(sample5-sign-verify PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(sample5-sign-verify PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample5-sign-verify)

target_link_libraries(sample5-sign-verify PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
