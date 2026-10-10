# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (certificate-import-certificate LANGUAGES CXX)

add_executable (
  certificate-import-certificate
  certificate_import_certificate.cpp
)
target_compile_features(certificate-import-certificate PRIVATE cxx_std_14)
set_target_properties(certificate-import-certificate PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(certificate-import-certificate PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault certificate-import-certificate)

target_link_libraries(certificate-import-certificate PRIVATE azure-security-keyvault-certificates azure-identity get-env-helper)
