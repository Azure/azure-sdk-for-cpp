# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.18)

project(azure-data-tables-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
  azure-data-tables-stress-test
  tables_stress_test.cpp
)

target_link_libraries(azure-data-tables-stress-test PRIVATE azure-data-tables)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
