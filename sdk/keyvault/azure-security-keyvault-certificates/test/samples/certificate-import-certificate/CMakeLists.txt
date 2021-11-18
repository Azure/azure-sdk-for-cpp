# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (certificate-import-certificate LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  certificate-import-certificate
  certificate_import_certificate.cpp
)

target_link_libraries(certificate-import-certificate PRIVATE azure-security-keyvault-certificates azure-identity )

# Add the sample to be run on CI. 
# CI pipeline reads the {service}-samples.txt and runs the binaries listed there.
file(APPEND ${CMAKE_BINARY_DIR}/keyvault-samples.txt "${CMAKE_CURRENT_BINARY_DIR}/certificate-import-certificate\n")
