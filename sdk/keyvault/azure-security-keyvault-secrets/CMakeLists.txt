# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-secrets LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(AzureVcpkg)
include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.0.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.0.0" REQUIRED)
  endif()
endif()

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_HEADER
    inc/azure/keyvault/secrets/dll_import_export.hpp
    inc/azure/keyvault/secrets/secret_client.hpp
    inc/azure/keyvault/keyvault_secrets.hpp
)

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE
    src/private/package_version.hpp
    src/secret_client.cpp
)

add_library(azure-security-keyvault-secrets ${AZURE_SECURITY_KEYVAULT_SECRETS_HEADER} ${AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE})

target_include_directories(
  azure-security-keyvault-secrets
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      # Disable dependency on azure-core until azure-core has released to vcpkg
      # ${azure-core-cpp_INCLUDE_DIRS}
)

# Disable dependency on azure-core until azure-core has released to vcpkg
# target_link_libraries(azure-security-keyvault-secrets PUBLIC Azure::azure-core)

# make sure that users can consume the project as a library.
add_library(Azure::azure-security-keyvault-secrets ALIAS azure-security-keyvault-secrets)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(azure-security-keyvault-secrets azure-security-keyvault-secrets azure-security-keyvault-secrets-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private//package_version.hpp")
generate_documentation(azure-security-keyvault-secrets ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-security-keyvault-secrets
    TEMPLATE
    "azure/azure-security-keyvault-secrets/dll_import_export.hpp"
  )

if(BUILD_TESTING)
    # tests
    add_subdirectory(test)
endif()
