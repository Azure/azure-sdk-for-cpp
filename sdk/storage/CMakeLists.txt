# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

project (azure-storage LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_STORAGE_SAMPLES "Build storage sample codes" ON)

if(BUILD_TESTING)
  add_executable(azure-storage-test)
  add_gtest(azure-storage-test)
endif()

if(BUILD_STORAGE_SAMPLES)
  add_executable(azure-storage-sample)
endif()

add_subdirectory(azure-storage-common)
add_subdirectory(azure-storage-blobs)
add_subdirectory(azure-storage-files-datalake)
add_subdirectory(azure-storage-files-shares)
