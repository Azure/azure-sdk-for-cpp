# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.12)

set(TARGET_NAME "azure-perf-stress-test")

project (${TARGET_NAME} LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable (
     ${TARGET_NAME}
     src/no_op_test.cpp
     )

target_link_libraries(${TARGET_NAME} PRIVATE azure-core azure-perf-stress)
