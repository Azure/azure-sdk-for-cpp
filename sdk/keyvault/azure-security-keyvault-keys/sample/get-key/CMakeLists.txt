# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-keys-sample-get-key LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  azure-security-keyvault-keys-sample-get-key
  main.cpp
)

target_link_libraries(azure-security-keyvault-keys-sample-get-key PRIVATE azure-security-keyvault-keys azure-identity)
