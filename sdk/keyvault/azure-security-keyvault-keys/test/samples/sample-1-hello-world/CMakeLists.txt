# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample-1-hello-world LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample-1-hello-world
  main.cpp
)

target_link_libraries(sample-1-hello-world PRIVATE azure-security-keyvault-keys azure-identity)
