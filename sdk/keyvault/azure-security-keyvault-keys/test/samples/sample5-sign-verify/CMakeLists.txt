# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (sample5-sign-verify LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample5-sign-verify
  sample5_sign_verify.cpp
)
create_per_service_target_build_for_sample(keyvault sample5-sign-verify)

target_link_libraries(sample5-sign-verify PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
