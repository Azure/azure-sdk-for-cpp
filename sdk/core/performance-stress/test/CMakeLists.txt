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

if(MSVC)
  # Since we could be built as either DLL or a static library, when we are built as static library,
  # linker does generate a bunch of warnings related to the use of dllimport attribute,
  # basically, suggesting that its use is unneccessary. We know that.
  # LNK4217: symbol 'symbol' defined in 'filename_1.obj' is imported by 'filename_2.obj' in function 'function'
  # LNK4286: symbol 'symbol' defined in 'filename_1.obj' is imported by 'filename_2.obj'
  target_link_options(azure-perf-stress-test PRIVATE /ignore:4217 /ignore:4286)
endif()
