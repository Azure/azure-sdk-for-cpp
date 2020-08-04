# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)
set(TARGET_NAME "azure-perf-stress")
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library (
  ${TARGET_NAME}
  src/placeholder.cpp
  )

target_include_directories (${TARGET_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/az_core>)

# make sure that users can consume the project as a library.
add_library (Azure::PerfStress ALIAS ${TARGET_NAME})

# Import azure-core to get a context type
target_link_libraries(${TARGET_NAME} PRIVATE azure-core)

add_subdirectory(test)
