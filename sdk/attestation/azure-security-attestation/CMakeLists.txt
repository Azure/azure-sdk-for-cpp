# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-security-attestation LANGUAGES CXX)

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
  AZURE_ATTESTATION_HEADER
    inc/azure/attestation/dll_import_export.hpp
    inc/azure/attestation/attestation_client.hpp
    inc/azure/attestation.hpp
)

set(
  AZURE_ATTESTATION_SOURCE
    src/private/package_version.hpp
    src/attestation_client.cpp
)

add_library(azure-security-attestation ${AZURE_ATTESTATION_HEADER} ${AZURE_ATTESTATION_SOURCE})

target_include_directories(
  azure-security-attestation
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      # Disable dependency on azure-core until azure-core has released to vcpkg
      # ${azure-core-cpp_INCLUDE_DIRS}
)

# Disable dependency on azure-core until azure-core has released to vcpkg
# target_link_libraries(azure-attestation PUBLIC Azure::azure-core)

# make sure that users can consume the project as a library.
add_library(Azure::azure-security-attestation ALIAS azure-security-attestation)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(attestation azure-security-attestation azure-security-attestation-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-security-attestation ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-security-attestation
    ATTESTATION
    "azure/attestation/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-security-attestation
  ATTESTATION
  "azure/attestation/rtti.hpp"
)

if(BUILD_TESTING)
    # tests
    add_subdirectory(test)
endif()
