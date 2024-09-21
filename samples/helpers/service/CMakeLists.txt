# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

cmake_minimum_required (VERSION 3.12)
project(service LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

add_library(
  service
    OBJECT
      inc/azure/service/client.hpp
      src/client.cpp
 )

 target_link_libraries(service PUBLIC azure-core)

 target_compile_definitions(service PRIVATE _azure_BUILDING_SAMPLES)

target_include_directories(service PUBLIC inc)
