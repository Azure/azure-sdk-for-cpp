# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project(azure-perf-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
  azure-perf-stress-test
    src/no_op_test.cpp
)

target_link_libraries(azure-perf-stress-test PRIVATE azure-core azure-perf-stress)
