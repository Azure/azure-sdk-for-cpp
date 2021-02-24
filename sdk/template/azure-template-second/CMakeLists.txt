# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-template-second LANGUAGES CXX)

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
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_TEMPLATE_HEADER
    inc/azure/template/second/dll_import_export.hpp
    inc/azure/template/second/template_client.hpp
    inc/azure/template/second/version.hpp
    inc/azure/template-second.hpp
)

set(
  AZURE_TEMPLATE_SOURCE
    src/template_second_client.cpp
    src/version.cpp
)

add_library(azure-template-second ${AZURE_TEMPLATE_HEADER} ${AZURE_TEMPLATE_SOURCE})

target_include_directories(
  azure-template-second
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      # Disable dependency on azure-core until azure-core has released to vcpkg
      # ${azure-core-cpp_INCLUDE_DIRS}
)

# Disable dependency on azure-core until azure-core has released to vcpkg
# target_link_libraries(azure-template-second PUBLIC Azure::azure-core)

# make sure that users can consume the project as a library.
add_library(Azure::azure-template-second ALIAS azure-template-second)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(template azure-template-second azure-template-second-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/template/second/version.hpp")
generate_documentation(azure-template-second ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-template-second
    TEMPLATE
    "azure/template/second/dll_import_export.hpp"
  )

if(BUILD_TESTING)
    # tests
    add_subdirectory(test)
endif()
