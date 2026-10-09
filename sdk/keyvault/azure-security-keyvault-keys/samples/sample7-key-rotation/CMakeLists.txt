# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample7-key-rotation LANGUAGES CXX)
if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14 OR CMAKE_CXX_STANDARD STREQUAL "98")
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample7-key-rotation
  sample7_key_rotation.cpp
)

target_compile_definitions(sample7-key-rotation PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample7-key-rotation)

target_link_libraries(sample7-key-rotation PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
