# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-template LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

find_package(azure-storage-common-cpp REQUIRED)

set(
  AZURE_TEMPLATE_HEADER
    inc/azure/template/dll_import_export.hpp
    inc/azure/template/template_client.hpp
    inc/azure/template.hpp
    inc/azure/storage/blobs/rest_client.hpp
)

set(
  AZURE_TEMPLATE_SOURCE
    src/private/package_version.hpp
    src/template_client.cpp
    src/rest_client.cpp
)

add_library(azure-template ${AZURE_TEMPLATE_HEADER} ${AZURE_TEMPLATE_SOURCE})

target_include_directories(
  azure-template
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-template PUBLIC Azure::azure-core Azure::azure-storage-common)

# make sure that users can consume the project as a library.
add_library(Azure::azure-template ALIAS azure-template)

