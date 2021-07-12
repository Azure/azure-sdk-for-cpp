# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-security-keyvault-secrets-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)

add_executable (
     azure-security-keyvault-secrets-test
     ut/macro_guard.cpp
     ut/secret_client_test.cpp
     )

target_link_libraries(azure-security-keyvault-secrets-test PRIVATE Azure::azure-template gtest_main)

if (MSVC)
    target_compile_options(azure-security-keyvault-secrets-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

gtest_discover_tests(azure-security-keyvault-secrets-test TEST_PREFIX azure-security-keyvault-secrets-test.)
