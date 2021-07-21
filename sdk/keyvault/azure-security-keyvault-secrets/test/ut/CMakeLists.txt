# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-secrets-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include(GoogleTest)

add_executable (
  azure-security-keyvault-secrets-test
    azure_security_keyvault_secrets_test.cpp
    macro_guard.cpp
    secret_client_test.cpp
    secret_get_client_deserialize_test.hpp
    secret_get_client_deserialize_test.cpp
    secret_set_parameters_serializer_test.cpp)

if (MSVC)
  target_compile_options(azure-security-keyvault-secrets-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

target_link_libraries(azure-security-keyvault-secrets-test PRIVATE azure-security-keyvault-secrets azure-identity gtest gmock)

# gtest_add_tests will scan the test from azure-core-test and call add_test
# for each test to ctest. This enables `ctest -r` to run specific tests directly.
gtest_discover_tests(azure-security-keyvault-secrets-test
  TEST_PREFIX azure-security-keyvault-secrets-unittest.
  NO_PRETTY_TYPES
  NO_PRETTY_VALUES
)
