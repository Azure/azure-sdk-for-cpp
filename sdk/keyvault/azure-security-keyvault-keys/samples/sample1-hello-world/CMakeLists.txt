# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (sample1-hello-world LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  sample1-hello-world
  sample1_hello_world.cpp
)

target_compile_definitions(sample1-hello-world PRIVATE _azure_BUILDING_SAMPLES)

create_per_service_target_build_for_sample(keyvault sample1-hello-world)

target_link_libraries(sample1-hello-world PRIVATE azure-security-keyvault-keys azure-identity get-env-helper)
