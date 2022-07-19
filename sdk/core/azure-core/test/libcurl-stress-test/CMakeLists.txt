# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.13)

project(azure-core-libcurl-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
  azure-core-libcurl-stress-test
  libcurl_stress_test.cpp
)

target_link_libraries(azure-core-libcurl-stress-test PRIVATE azure-core)

create_map_file(azure-core-libcurl-stress-test azure-core-libcurl-stress-test.map)
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/Dockerfile
  DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)
