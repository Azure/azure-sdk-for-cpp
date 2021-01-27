# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-identity LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(AzureVcpkg)
include(AzureVersion)
include(CodeCoverage)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_IDENTITY_HEADER
    inc/azure/identity/client_secret_credential.hpp
    inc/azure/identity/environment_credential.hpp
    inc/azure/identity/dll_import_export.hpp
    inc/azure/identity/version.hpp
    inc/azure/identity.hpp
)

set(
  AZURE_IDENTITY_SOURCE
    src/client_secret_credential.cpp
    src/environment_credential.cpp
    src/version.cpp
)

add_library(azure-identity ${AZURE_IDENTITY_HEADER} ${AZURE_IDENTITY_SOURCE})

# make sure that users can consume the project as a library.
add_library(Azure::azure-identity ALIAS azure-identity)

# Uncomment once identity have tests
# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
# create_code_coverage(identity azure-identity azure-identity-test)

target_include_directories(
  azure-identity
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      ${azure-core-cpp_INCLUDE_DIRS}
)

target_link_libraries(azure-identity PUBLIC Azure::azure-core)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/identity/version.hpp")
generate_documentation(azure-identity ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-identity
    IDENTITY
    "azure/identity/dll_import_export.hpp"
  )
