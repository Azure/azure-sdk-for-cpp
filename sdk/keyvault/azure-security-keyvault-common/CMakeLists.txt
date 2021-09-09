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
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.2.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.2.0" REQUIRED)
  endif()
endif()

set(
  AZURE_KEYVAULT_COMMON_SOURCE
    src/private/package_version.hpp
)

add_library(
    azure-security-keyvault-common
    ${AZURE_KEYVAULT_COMMON_SOURCE}
)
add_library(Azure::azure-security-keyvault-common ALIAS azure-security-keyvault-common)

target_include_directories(
    azure-security-keyvault-common
      PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-security-keyvault-common PUBLIC Azure::azure-core)

if(WIN32)
    target_link_libraries(azure-security-keyvault-common PRIVATE bcrypt crypt32)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-security-keyvault-common PRIVATE OpenSSL::SSL)
endif()

if(MSVC)
    # C28020 and C28204 are introduced by nlohmann/json
    target_compile_options(azure-security-keyvault-common PUBLIC /wd28204 /wd28020)
endif()

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-common azure-security-keyvault-common-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
#generate_documentation(azure-security-keyvault-common ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)
    # tests
  if (NOT AZ_ALL_LIBRARIES)
    include(AddGoogleTest)
    enable_testing ()
  endif()

  add_subdirectory(test/ut)
endif()

az_vcpkg_export(
    azure-security-keyvault-common
    SECURITY_KEYVAULT_COMMON
    "azure/keyvault/common/dll_import_export.hpp"
  )
