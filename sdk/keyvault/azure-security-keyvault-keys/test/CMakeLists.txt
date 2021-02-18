# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-keys-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

################## Unit Tests ##########################
add_executable (
  azure-security-keyvault-keys-test
  key_client_test.cpp
  main.cpp
  mocked_transport_adapter_test.hpp
  telemetry_header_test.cpp
)

if (MSVC)
  target_compile_options(azure-security-keyvault-keys-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-security-keyvault-keys-test PRIVATE azure-security-keyvault-keys azure-identity gtest gmock)

# gtest_add_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_add_tests(TARGET azure-security-keyvault-keys-test
     TEST_PREFIX azure-security-keyvault-keys-unittest.)


################## Live Tests ##########################
add_executable (
  azure-security-keyvault-keys-test-live
  key_client_test_live.cpp
  main.cpp
  key_client_base_test.hpp
)

if (MSVC)
  target_compile_options(azure-security-keyvault-keys-test-live PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-security-keyvault-keys-test-live PRIVATE azure-security-keyvault-keys azure-identity gtest gmock)

# gtest_discover_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-security-keyvault-keys-test-live
  TEST_PREFIX azure-security-keyvault-keys-livetest.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
)

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(performance)
endif()
