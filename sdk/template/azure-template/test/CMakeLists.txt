# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-template-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_definitions(AZURE_TEST_DATA_PATH="${CMAKE_BINARY_DIR}")

add_executable (
     azure-template-test
     ut/template_test.cpp
     )

target_link_libraries(azure-template-test PRIVATE Azure::azure-template)

if (MSVC)
    target_compile_options(azure-template-test PUBLIC /wd6326 /wd26495 /wd26812)
endif()

add_gtest(azure-template-test)

