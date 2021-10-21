# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (certificate-GetCertificates LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  certificate-GetCertificates
  certificate-GetCertificates.cpp
)

target_link_libraries(certificate-GetCertificates PRIVATE azure-security-keyvault-certificates azure-identity )
