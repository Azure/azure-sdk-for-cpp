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
  AZURE_IDENTITY_HEADER
    inc/azure/identity/client_secret_credential.hpp
    inc/azure/identity/dll_import_export.hpp
    inc/azure/identity/environment_credential.hpp
    inc/azure/identity/managed_identity_credential.hpp
    inc/azure/identity.hpp
)

set(
  AZURE_IDENTITY_SOURCE
    src/private/environment.hpp
    src/private/managed_identity_source.hpp
    src/private/package_version.hpp
    src/private/token_credential_impl.hpp
    src/client_secret_credential.cpp
    src/environment.cpp
    src/environment_credential.cpp
    src/managed_identity_credential.cpp
    src/managed_identity_source.cpp
    src/token_credential_impl.cpp
)

add_library(azure-identity ${AZURE_IDENTITY_HEADER} ${AZURE_IDENTITY_SOURCE})

# make sure that users can consume the project as a library.
add_library(Azure::azure-identity ALIAS azure-identity)

create_code_coverage(identity azure-identity azure-identity-test "tests?/*;samples?/*")

target_include_directories(
  azure-identity
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      ${azure-core-cpp_INCLUDE_DIRS}
)

target_link_libraries(azure-identity PUBLIC Azure::azure-core)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-identity ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-identity
    IDENTITY
    "azure/identity/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-identity
  IDENTITY
  "azure/identity/rtti.hpp"
)

if(BUILD_TESTING)
  # tests
  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()

  add_subdirectory(test/e2e)
  add_subdirectory(test/live)
  add_subdirectory(test/ut)
endif()

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(test/perf)
endif()

if (AZ_ALL_LIBRARIES)
  add_subdirectory(samples)
endif()
