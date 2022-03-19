# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (certificate-get-certificates LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  certificate-get-certificates
  certificate_get_certificates.cpp
)
create_per_service_target_build(certificate-get-certificates keyvault true)

target_link_libraries(certificate-get-certificates PRIVATE azure-security-keyvault-certificates azure-identity get-env-helper)
