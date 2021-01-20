# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-common-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

add_executable (
  azure-security-keyvault-common-test
  pipeline_test.cpp
  main.cpp
)

if (MSVC)
  target_compile_options(azure-security-keyvault-common-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-security-keyvault-common-test PUBLIC azure-security-keyvault-common gtest gmock)

# gtest_add_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_add_tests(TARGET azure-security-keyvault-common-test
     TEST_PREFIX azure-security-keyvault-common-test.)
