# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-performance-stress LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_PERFORMANCE_HEADER
  inc/azure/performance-stress/argagg.hpp
  inc/azure/performance-stress/base_test.hpp
  inc/azure/performance-stress/dynamic_test_options.hpp
  inc/azure/performance-stress/options.hpp
  inc/azure/performance-stress/program.hpp
  inc/azure/performance-stress/test_metadata.hpp
  inc/azure/performance-stress/test.hpp
  inc/azure/performance-stress/test_options.hpp
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

if (MSVC)
  #argagg warnings
  target_compile_options(azure-performance-stress PUBLIC /wd28020 /wd28204)
endif()

# make sure that users can consume the project as a library.
add_library (Azure::PerfStress ALIAS azure-performance-stress)
target_link_libraries(azure-performance-stress PRIVATE azure-core)

set_target_properties(azure-performance-stress PROPERTIES FOLDER "Core")

add_subdirectory(test)
