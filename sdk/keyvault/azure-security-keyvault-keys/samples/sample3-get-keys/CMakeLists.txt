# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample3-get-keys LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample3-get-keys
  sample3_get_keys.cpp
)
create_per_service_target_build_for_sample(keyvault sample3-get-keys DISABLE_RUN)

target_link_libraries(sample3-get-keys PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
