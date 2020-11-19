# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-core-test")
project(azure-core-test LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library (
  azure-core-test
  src/interceptor_manager.cpp
  src/playback_http_transport.cpp
  src/record_policy.cpp
  src/test_base.cpp
  )

target_include_directories (azure-core-test PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/az_core_test>)

# make sure that users can consume the project as a library.
add_library (Azure::Core::Test ALIAS azure-core-test)

target_link_libraries(azure-core-test PRIVATE azure-core gtest)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/core/test/version.hpp")
generate_documentation(azure-core-test ${AZ_LIBRARY_VERSION})
