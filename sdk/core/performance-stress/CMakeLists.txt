# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-perf-stress LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library (
  azure-perf-stress
    src/placeholder.cpp
)

target_include_directories(
  azure-perf-stress
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include/az_core>
)

# make sure that users can consume the project as a library.
add_library (Azure::PerfStress ALIAS azure-perf-stress)
target_link_libraries(azure-perf-stress PRIVATE azure-core)

add_subdirectory(test)
