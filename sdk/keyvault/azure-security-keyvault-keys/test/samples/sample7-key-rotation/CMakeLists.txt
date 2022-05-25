# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample7-key-rotation LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample7-key-rotation
  sample7_key_rotation.cpp
)
create_per_service_target_build_for_sample(keyvault sample7-key-rotation)

target_link_libraries(sample7-key-rotation PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
