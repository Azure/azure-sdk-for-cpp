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
  find_package(azure-security-keyvault-common-cpp "4.0.0" CONFIG QUIET)
  if(NOT azure-security-keyvault-common-cpp_FOUND)
    find_package(azure-security-keyvault-common-cpp "4.0.0" REQUIRED)
  endif()
endif()

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_HEADER
    inc/azure/keyvault/secrets/dll_import_export.hpp
    inc/azure/keyvault/secrets/secret_client.hpp
    inc/azure/keyvault/secrets/keyvault_secret.hpp
    inc/azure/keyvault/keyvault_secrets.hpp
)

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE
    src/private/package_version.hpp
    
    src/secret_client.cpp
)

add_library(azure-security-keyvault-secrets ${AZURE_SECURITY_KEYVAULT_SECRETS_HEADER} ${AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE} "inc/azure/keyvault/secrets/keyvault_secret.hpp" "src/private/secret_constants.hpp" "src/private/secret_serializers.hpp" "src/secret_serializers.cpp" "inc/azure/keyvault/secrets/keyvault_secret_properties.hpp")
add_library(Azure::azure-security-keyvault-secrets ALIAS azure-security-keyvault-secrets)

target_include_directories(
  azure-security-keyvault-secrets
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-security-keyvault-secrets PUBLIC PUBLIC Azure::azure-security-keyvault-common)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault-secrets azure-security-keyvault-secrets azure-security-keyvault-secrets-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-security-keyvault-secrets ${AZ_LIBRARY_VERSION})
if(BUILD_TESTING)

  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
  add_subdirectory(test/sample)
endif()

if (BUILD_PERFORMANCE_TESTS)
  #add_subdirectory(test/perf)
endif()

if(BUILD_SAMPLES)
  #add_subdirectory(test/samples)
endif()

az_vcpkg_export(
    azure-security-keyvault-secrets
    SECURITY_KEYVAULT_KEYS
    "azure/keyvault/secrets/dll_import_export.hpp"
  )
