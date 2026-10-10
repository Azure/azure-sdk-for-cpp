# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

project(azure-data-tables-stress-test LANGUAGES CXX)

add_executable(
  azure-data-tables-stress-test
  tables_stress_test.cpp
)
target_compile_features(azure-data-tables-stress-test PRIVATE cxx_std_14)
set_target_properties(azure-data-tables-stress-test PROPERTIES CXX_STANDARD_REQUIRED ON)

target_compile_definitions(azure-data-tables-stress-test PRIVATE _azure_BUILDING_TESTS)

target_link_libraries(azure-data-tables-stress-test PRIVATE azure-data-tables)

file(COPY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
