# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required(VERSION 3.13)

az_build_shared_lib_test(azure-core CORE)

project(azure-core-libcurl-stress-test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(
  azure-core-libcurl-stress-test
  libcurl_stress_test.cpp
)

target_link_libraries(azure-core-libcurl-stress-test PRIVATE azure-core)

target_compile_definitions(azure-core-libcurl-stress-test PRIVATE _azure_BUILDING_TESTS)

create_map_file(azure-core-libcurl-stress-test azure-core-libcurl-stress-test.map)
file(COPY ${CMAKE_CURRENT_BINARY_DIR}
  DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/bin)
