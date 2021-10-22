# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (certificate-ImportCertificate LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  certificate-ImportCertificate
  certificate-ImportCertificate.cpp
 "certificate-ImportCertificate.hpp")

target_link_libraries(certificate-ImportCertificate PRIVATE azure-security-keyvault-certificates azure-identity )
