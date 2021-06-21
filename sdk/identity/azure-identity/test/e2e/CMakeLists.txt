# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-identity-e2e-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  azure-identity-e2e-test
    azure_identity_e2e_test.cpp
)

target_link_libraries(azure-identity-e2e-test PRIVATE azure-identity)
