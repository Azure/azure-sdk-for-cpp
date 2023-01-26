# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-administration-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

# Export the test folder for recordings access.
add_compile_definitions(AZURE_TEST_RECORDING_DIR="${CMAKE_CURRENT_LIST_DIR}")
add_compile_definitions(AZURE_TEST_ASSETS_DIR="${CMAKE_CURRENT_LIST_DIR}/../../../")

add_executable (
  azure-security-keyvault-administration-test
    macro_guard.cpp
    settings_client_test.cpp
    settings_client_base_test.hpp)

create_per_service_target_build(keyvault azure-security-keyvault-administration-test)

create_map_file(azure-security-keyvault-administration-test azure-security-keyvault-administration-test.map)

if (MSVC)
  target_compile_options(azure-security-keyvault-administration-test PUBLIC /wd6326 /wd26495 /wd26812 /wd4389)
endif()

target_link_libraries(azure-security-keyvault-administration-test PRIVATE azure-security-keyvault-administration azure-identity azure-core-test-fw gtest gtest_main gmock)

# Adding private headers so we can test the private APIs with no relative paths include.
target_include_directories (azure-security-keyvault-administration-test PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../src>)

# gtest_add_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-security-keyvault-administration-test
  TEST_PREFIX azure-security-keyvault-administration.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
  DISCOVERY_TIMEOUT 600)
