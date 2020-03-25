# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

include(AddGoogleTest)

project (azure-core-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
     azure-core-test
     main.cpp 
)

target_link_libraries(azure-core-test PRIVATE azure-core gtest_main)
add_gtest(azure-core-test)

