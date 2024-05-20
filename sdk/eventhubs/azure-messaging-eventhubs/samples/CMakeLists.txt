# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.18)

if(MSVC)
  add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(basic-operations)
add_subdirectory(produce-events)
add_subdirectory(consume-events)
