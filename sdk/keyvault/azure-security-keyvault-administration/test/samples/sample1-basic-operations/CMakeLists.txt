# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample1-administration LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample1-administration
  sample1_administration.cpp
)

target_compile_definitions(sample1-administration PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample1-administration)

target_link_libraries(sample1-administration PRIVATE azure-security-keyvault-administration azure-identity get-env-helper)
