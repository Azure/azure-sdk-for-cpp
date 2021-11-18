# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-certificates LANGUAGES CXX)

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
include(AzureConfigRTTI)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.2.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.2.0" REQUIRED)
  endif()
endif()

set(
  AZURE_KEYVAULT_CERTIFICATES_HEADER
    inc/azure/keyvault/certificates/certificate_client.hpp
    inc/azure/keyvault/certificates/certificate_client_models.hpp
    inc/azure/keyvault/certificates/certificate_client_options.hpp
    inc/azure/keyvault/certificates/certificate_client_operations.hpp
)

set(
  AZURE_KEYVAULT_CERTIFICATES_SOURCE
    src/certificate_client.cpp
    src/certificate_serializers.cpp
    src/keyvault_certificates_common_request.cpp
    src/certificate_client_operations.cpp
    src/certificate_client_paged_response.cpp
)

add_library(azure-security-keyvault-certificates
  ${AZURE_KEYVAULT_CERTIFICATES_HEADER} ${AZURE_KEYVAULT_CERTIFICATES_SOURCE}
)
add_library(Azure::azure-security-keyvault-certificates ALIAS azure-security-keyvault-certificates)

target_include_directories(
  azure-security-keyvault-certificates
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

# Include shared source code
# NOTE: Use shared-code only within .cpp files. DO NEVER consume the shared-code from header files.
target_include_directories(
  azure-security-keyvault-certificates
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../azure-security-keyvault-shared/inc>
)

target_link_libraries(azure-security-keyvault-certificates PUBLIC Azure::azure-core)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-certificates azure-security-keyvault-certificates-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-security-keyvault-certificates ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)

  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
endif()

# if (BUILD_PERFORMANCE_TESTS)
#   add_subdirectory(test/perf)
# endif()

if(BUILD_SAMPLES)
   add_subdirectory(test/samples)
endif()

az_vcpkg_export(
    azure-security-keyvault-certificates
    SECURITY_KEYVAULT_CERTIFICATES
    "azure/keyvault/certificates/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-security-keyvault-certificates
  SECURITY_KEYVAULT_CERTIFICATES
  "azure/keyvault/certificates/rtti.hpp"
)
