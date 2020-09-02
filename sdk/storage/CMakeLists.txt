# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

project (azure-storage LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_STORAGE_SAMPLES "Build storage sample codes" ON)


if(MSVC)
    add_compile_definitions(NOMINMAX)
endif()


add_executable(azure-storage-test)
if(BUILD_TESTING)
    set_target_properties(azure-storage-test PROPERTIES EXCLUDE_FROM_ALL FALSE)
    add_gtest(azure-storage-test)
else()
    set_target_properties(azure-storage-test PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()


add_executable(azure-storage-sample)
if(BUILD_STORAGE_SAMPLES)
    set_target_properties(azure-storage-sample PROPERTIES EXCLUDE_FROM_ALL FALSE)
else()
    set_target_properties(azure-storage-sample PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()


add_subdirectory(azure-storage-common)
add_subdirectory(azure-storage-blobs)
# add_subdirectory(azure-storage-files-datalake)
# add_subdirectory(azure-storage-files-shares)
