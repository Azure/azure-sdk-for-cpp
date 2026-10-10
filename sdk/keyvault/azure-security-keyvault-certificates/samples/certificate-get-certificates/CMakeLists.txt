# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (certificate-get-certificates LANGUAGES CXX)

add_executable (
  certificate-get-certificates
  certificate_get_certificates.cpp
)
target_compile_features(certificate-get-certificates PRIVATE cxx_std_14)
set_target_properties(certificate-get-certificates PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(certificate-get-certificates PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault certificate-get-certificates)

target_link_libraries(certificate-get-certificates PRIVATE azure-security-keyvault-certificates azure-identity get-env-helper)
