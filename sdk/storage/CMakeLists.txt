# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.15)

project (azure-storage LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(BUILD_STORAGE_SAMPLES "Build sample codes" ON)

if(MSVC)
    add_compile_definitions(NOMINMAX)
endif()

add_subdirectory(azure-storage-common)
add_subdirectory(azure-storage-blobs)
add_subdirectory(azure-storage-files-datalake)
add_subdirectory(azure-storage-files-shares)

# TODO: eng/sys teams is going to fix this
generate_documentation(azure-storage 1.0.0-preview.1)
