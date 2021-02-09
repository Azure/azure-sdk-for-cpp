# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-template2-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

include(GoogleTest)

add_executable (
     azure-template2-test
     ut/template_test.cpp
     )

target_link_libraries(azure-template2-test PRIVATE Azure::azure-template2 gtest_main)

if (MSVC)
    target_compile_options(azure-template2-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

gtest_discover_tests(azure-template2-test TEST_PREFIX azure-template2.)

