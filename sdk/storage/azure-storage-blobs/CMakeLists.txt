# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-blobs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_vcpkg)
include(az_version)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-storage-common-cpp CONFIG QUIET)
  if(NOT azure-storage-common-cpp_FOUND)
    find_package(azure-storage-common-cpp REQUIRED)
  endif()
endif()

add_library(
  azure-storage-blobs
    src/append_blob_client.cpp
    src/blob_batch_client.cpp
    src/blob_client.cpp
    src/blob_container_client.cpp
    src/blob_sas_builder.cpp
    src/blob_service_client.cpp
    src/block_blob_client.cpp
    src/page_blob_client.cpp
)

# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-blobs ALIAS azure-storage-blobs)

target_include_directories(
  azure-storage-blobs
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      ${azure-storage-common-cpp_INCLUDE_DIRS}
)

target_link_libraries(azure-storage-blobs PUBLIC Azure::azure-storage-common)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/blobs/version.hpp")
generate_documentation(azure-storage-blobs ${AZ_LIBRARY_VERSION})

az_vcpkg_export(azure-storage-blobs)
