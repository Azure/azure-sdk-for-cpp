# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.13)

project (macros-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  macros-build
  macros_build.cpp
  macros_build.hpp)

target_compile_definitions(macros-build PRIVATE _azure_BUILDING_TESTS)

create_per_service_target_build_for_sample(keyvault macros-build)

target_link_libraries(macros-build PRIVATE azure-security-keyvault-keys)
