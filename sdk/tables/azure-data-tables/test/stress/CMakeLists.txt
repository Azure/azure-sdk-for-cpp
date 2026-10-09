# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

project(azure-data-tables-stress-test LANGUAGES CXX)
if(NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 14 OR CMAKE_CXX_STANDARD STREQUAL "98")
  set(CMAKE_CXX_STANDARD 14)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
  azure-data-tables-stress-test
  tables_stress_test.cpp
)

target_compile_definitions(azure-data-tables-stress-test PRIVATE _azure_BUILDING_TESTS)

target_link_libraries(azure-data-tables-stress-test PRIVATE azure-data-tables)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
