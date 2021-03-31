# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-identity-livetest LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

add_executable (
  azure-identity-livetest
  client_secret_credential.cpp
  common.hpp
  environment_credential.cpp
  main.cpp
)

if (MSVC)
  target_compile_options(azure-identity-livetest PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-identity-livetest PRIVATE azure-identity gtest gmock)

# gtest_discover_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-identity-livetest
  TEST_PREFIX azure-identity-livetest.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
)
