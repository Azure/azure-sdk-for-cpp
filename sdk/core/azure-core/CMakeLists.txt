# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

project (azure-core LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library (
  azure-core
  src/context
  src/credentials/credentials
  src/http/http
  src/http/request
  src/http/response
  )

target_include_directories (azure-core PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/az_core>)

# make sure that users can consume the project as a library.
add_library (azure::core ALIAS azure-core)
