# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)
set(TARGET_NAME "azure-template")
project(azure-template LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library (
  azure-template
  src/version.cpp
  src/template_client.cpp
  )

target_include_directories (azure-template PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>)

# make sure that users can consume the project as a library.
add_library (Azure::Template ALIAS azure-template)

if(BUILD_TESTING)
    # tests
    add_subdirectory(test)
endif()

file(READ "version.txt" package_version)
generate_documentation(${TARGET_NAME} "${}.${}")
