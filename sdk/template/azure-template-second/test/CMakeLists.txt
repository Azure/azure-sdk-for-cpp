# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-template-second-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)

add_executable (
     azure-template-second-test
     ut/macro_guard.cpp
     ut/template_test.cpp
     )

target_link_libraries(azure-template-second-test PRIVATE Azure::azure-template-second gtest_main)

if (MSVC)
    target_compile_options(azure-template-second-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

gtest_discover_tests(azure-template-second-test TEST_PREFIX azure-template-second.)
