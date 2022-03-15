# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-perf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(AzureTransportAdapters)

set(
  AZURE_PERFORMANCE_HEADER
  inc/azure/perf/argagg.hpp
  inc/azure/perf/base_test.hpp
  inc/azure/perf/dynamic_test_options.hpp
  inc/azure/perf/options.hpp
  inc/azure/perf/program.hpp
  inc/azure/perf/random_stream.hpp
  inc/azure/perf/test_metadata.hpp
  inc/azure/perf/test.hpp
  inc/azure/perf/test_options.hpp
)

set(
  AZURE_PERFORMANCE_SOURCE
  src/arg_parser.cpp
  src/base_test.cpp
  src/options.cpp
  src/program.cpp
  src/random_stream.cpp
)

add_library(azure-perf ${AZURE_PERFORMANCE_HEADER} ${AZURE_PERFORMANCE_SOURCE})

target_include_directories(
  azure-perf
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include/az_core>
)

if (MSVC)
  #argagg warnings
  target_compile_options(azure-perf PUBLIC /wd28020 /wd28204)
endif()

# make sure that users can consume the project as a library.
add_library (Azure::Perf ALIAS azure-perf)
target_link_libraries(azure-perf PUBLIC azure-core)
create_map_file(azure-perf azure-perf.map)

set_target_properties(azure-perf PROPERTIES FOLDER "Core")

add_subdirectory(test)

# Give a warning about winHTTP and insecure
if(BUILD_TRANSPORT_WINHTTP)
  message(WARNING "Note. WInHTTP does not support running performance tests with `insecure` mode enabled. Use libcurl transport adapter if you want to use insecure mode. ")
endif()
