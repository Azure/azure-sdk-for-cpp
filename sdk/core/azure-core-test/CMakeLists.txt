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
  src/test_proxy_policy.cpp
  src/test_base.cpp
  src/test_proxy_manager.cpp
)

add_library (
  azure-core-test-fw
  ${AZURE_CORE_TEST_HEADER}
  ${AZURE_CORE_TEST_SOURCE}
  )

# Additional test files to be copied to the output directory.
set(TEST_ADDITIONAL_FILES 
    ${CMAKE_CURRENT_LIST_DIR}/../../../eng/scripts/Start-TestProxy.ps1
    ${CMAKE_CURRENT_LIST_DIR}/../../../eng/scripts/Stop-TestProxy.ps1
)

add_custom_command(TARGET azure-core-test-fw POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${TEST_ADDITIONAL_FILES} ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${TEST_ADDITIONAL_FILES}
    COMMENT 'Copying non-source output files')

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
