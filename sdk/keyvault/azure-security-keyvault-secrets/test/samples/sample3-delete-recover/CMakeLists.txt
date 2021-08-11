# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample3-delete-recover LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample3-delete-recover
  sample.cpp
)

if (MSVC)
  target_compile_options(sample3-delete-recover PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(sample3-delete-recover PRIVATE azure-security-keyvault-secrets azure-identity)
