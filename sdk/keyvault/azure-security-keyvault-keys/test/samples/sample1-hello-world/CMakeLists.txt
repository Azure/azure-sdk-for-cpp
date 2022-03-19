# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample1-hello-world LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample1-hello-world
  sample1_hello_world.cpp
)

create_per_service_target_build(sample1-hello-world keyvault true)

target_link_libraries(sample1-hello-world PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
