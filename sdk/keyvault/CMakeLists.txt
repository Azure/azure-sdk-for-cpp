# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(BUILD_TESTING)
  # define a symbol that enables some test hooks in code
  add_compile_definitions(TESTING_BUILD)
  add_subdirectory(cleanup)
endif()

add_subdirectory(azure-security-keyvault-keys)
add_subdirectory(azure-security-keyvault-secrets)
add_subdirectory(azure-security-keyvault-certificates)
