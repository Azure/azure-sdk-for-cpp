# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample1 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample1
  sample.cpp
)

if (MSVC)
  target_compile_options(sample1 PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(sample1 PRIVATE azure-security-keyvault-secrets azure-identity)
