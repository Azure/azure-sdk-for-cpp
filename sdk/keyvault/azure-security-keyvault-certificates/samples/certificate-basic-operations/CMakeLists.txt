# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (certificate-basic-operations LANGUAGES CXX)

add_executable (
  certificate-basic-operations
  certificate_basic_operations.cpp
)

target_compile_definitions(certificate-basic-operations PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault certificate-basic-operations)

target_link_libraries(certificate-basic-operations PRIVATE azure-security-keyvault-certificates azure-identity get-env-helper)
