# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.12)
project(get-env-helper LANGUAGES CXX)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(
  get-env-helper
    OBJECT
      inc/get_env.hpp
      src/get_env.cpp
)
target_compile_features(get-env-helper PRIVATE cxx_std_14)
set_target_properties(get-env-helper PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(get-env-helper PRIVATE _azure_BUILDING_SAMPLES)

target_include_directories(get-env-helper PUBLIC inc)
