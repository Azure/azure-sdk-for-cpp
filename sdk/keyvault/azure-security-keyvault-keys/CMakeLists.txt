# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-keys LANGUAGES CXX)

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
  find_package(azure-security-keyvault-common-cpp CONFIG QUIET)
  if(NOT azure-security-keyvault-common-cpp_FOUND)
    find_package(azure-security-keyvault-common-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_KEYVAULT_KEYS_HEADER
    inc/azure/keyvault/keys/delete_key_operation.hpp
    inc/azure/keyvault/keys/deleted_key.hpp
    inc/azure/keyvault/keys/json_web_key.hpp
    inc/azure/keyvault/keys/key_client.hpp
    inc/azure/keyvault/keys/key_constants.hpp
    inc/azure/keyvault/keys/key_create_options.hpp
    inc/azure/keyvault/keys/key_client_options.hpp
    inc/azure/keyvault/keys/key_operation.hpp
    inc/azure/keyvault/keys/key_properties.hpp
    inc/azure/keyvault/keys/key_release_policy.hpp
    inc/azure/keyvault/keys/key_request_parameters.hpp
    inc/azure/keyvault/keys/key_type.hpp
    inc/azure/keyvault/keys/key_vault_key.hpp
    inc/azure/keyvault/keys/version.hpp
)

set(
  AZURE_KEYVAULT_KEYS_SOURCE
    src/delete_key_operation.cpp
    src/deleted_key.cpp
    src/key_client.cpp
    src/key_request_parameters.cpp
    src/key_type.cpp
    src/key_vault_key.cpp
)

add_library(azure-security-keyvault-keys
  ${AZURE_KEYVAULT_KEYS_HEADER} ${AZURE_KEYVAULT_KEYS_SOURCE}
)
add_library(Azure::azure-security-keyvault-keys ALIAS azure-security-keyvault-keys)

target_include_directories(
  azure-security-keyvault-keys
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-security-keyvault-keys PUBLIC Azure::azure-security-keyvault-common)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-keys azure-security-keyvault-keys-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/keyvault/keys/version.hpp")
generate_documentation(azure-security-keyvault-keys ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(sample)
endif()

az_vcpkg_export(
    azure-security-keyvault-keys
    SECURITY_KEYVAULT_KEYS
    "azure/keyvault/keys/dll_import_export.hpp"
  )
