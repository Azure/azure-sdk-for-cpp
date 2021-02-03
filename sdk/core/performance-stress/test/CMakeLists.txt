# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-performance-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Name the binary to be created.
add_executable (
  azure-performance-stress-test
    src/main.cpp
)

# Include the headers from the project.
target_include_directories(
  azure-performance-stress-test
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
)

# link the `azure-performance-stress` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-performance-stress-test PRIVATE azure-core azure-performance-stress)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-performance-stress-test PROPERTIES FOLDER "Tests")
