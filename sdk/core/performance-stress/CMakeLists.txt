# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-performance-stress LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_PERFORMANCE_HEADER
  inc/azure/performance-stress/base_test.hpp
  inc/azure/performance-stress/options.hpp
  inc/azure/performance-stress/program.hpp
  inc/azure/performance-stress/test.hpp
)

set(
  AZURE_PERFORMANCE_SOURCE
  src/arg_parser.cpp
  src/options.cpp
  src/program.cpp
)

add_library(azure-performance-stress ${AZURE_PERFORMANCE_HEADER} ${AZURE_PERFORMANCE_SOURCE})

target_include_directories(
  azure-performance-stress
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include/az_core>
)

# make sure that users can consume the project as a library.
add_library (Azure::PerfStress ALIAS azure-performance-stress)
target_link_libraries(azure-performance-stress PRIVATE azure-core)

add_subdirectory(test)
