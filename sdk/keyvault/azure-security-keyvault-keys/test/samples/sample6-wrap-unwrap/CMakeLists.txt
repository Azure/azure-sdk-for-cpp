# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample6-wrap-unwrap LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample6-wrap-unwrap
  sample6_wrap_unwrap.cpp
)
create_per_service_target_build(sample6-wrap-unwrap keyvault true)

target_link_libraries(sample6-wrap-unwrap PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
