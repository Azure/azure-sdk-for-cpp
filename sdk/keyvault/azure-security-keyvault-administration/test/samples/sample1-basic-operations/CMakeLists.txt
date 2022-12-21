# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample1-administration LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample1-administration
  sample1_administration.cpp
)
create_per_service_target_build_for_sample(keyvault sample1-administration)

target_link_libraries(sample1-administration PRIVATE azure-security-keyvault-administration azure-identity get-env-helper)
