# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.12)
project(get-env-helper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(
  get-env-helper
    OBJECT
      inc/get_env.hpp
      src/get_env.cpp
)

target_compile_definitions(get-env-helper PRIVATE _azure_BUILDING_SAMPLES)

target_include_directories(get-env-helper PUBLIC inc)
