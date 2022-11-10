# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-core-test-fw")
project(azure-core-test-fw LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(
  AZURE_CORE_TEST_HEADER
  inc/azure/core/test/network_models.hpp
  inc/azure/core/test/test_base.hpp
  inc/azure/core/test/test_context_manager.hpp
  inc/azure/core/test/test_proxy_manager.hpp
)

set(
  AZURE_CORE_TEST_SOURCE
  src/private/package_version.hpp
  
  src/record_test_proxy_policy.cpp
  src/test_base.cpp
  src/test_proxy_manager.cpp
)

add_library (
  azure-core-test-fw
  ${AZURE_CORE_TEST_HEADER}
  ${AZURE_CORE_TEST_SOURCE}
   "src/test_proxy_manager.cpp" "src/record_test_proxy_policy.cpp")

if (MSVC)
  # - C6326: Google comparisons 
  target_compile_options(azure-core-test-fw PUBLIC /wd6326)
endif()

target_include_directories (azure-core-test-fw
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
    $<INSTALL_INTERFACE:include/az_core_test>)

# make sure that users can consume the project as a library.
add_library (Azure::Core::Test ALIAS azure-core-test-fw)
target_link_libraries(azure-core-test-fw PRIVATE azure-core Azure::azure-identity gtest)
create_map_file(azure-core-test-fw azure-core-test-fw.map)