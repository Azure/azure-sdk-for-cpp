# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(AzureVcpkg)
include(AzureVersion)
include(AzureGlobalCompileOptions)

az_vcpkg_integrate()

set(
  AZURE_KEYVAULT_COMMON_SOURCE
    src/private/package_version.hpp
)

add_library(
    azure-security-keyvault-common
      inc/azure/keyvault/common/dll_import_export.hpp
      src/private/package_version.hpp
)
add_library(Azure::azure-security-keyvault-common ALIAS azure-security-keyvault-common)

target_include_directories(
    azure-security-keyvault-common
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:include>
)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")

az_vcpkg_export(
    azure-security-keyvault-common
    SECURITY_KEYVAULT_COMMON
    "azure/keyvault/common/dll_import_export.hpp"
  )
