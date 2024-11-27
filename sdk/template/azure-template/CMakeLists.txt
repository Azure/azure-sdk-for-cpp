# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-template LANGUAGES CXX)

include(AzureTransportAdapters)
include(AzureBuildTargetForCI)
include(AzureVersion)
include(AzureCodeCoverage)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)
# Add create_map_file function
include(CreateMapFile)


if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_TEMPLATE_HEADER
    inc/azure/template.hpp
    inc/azure/template/dll_import_export.hpp
    inc/azure/template/rtti.hpp
    inc/azure/template/template_client.hpp
)

set(
  AZURE_TEMPLATE_SOURCE
    src/private/package_version.hpp
    src/template_client.cpp
)

add_library(azure-template ${AZURE_TEMPLATE_HEADER} ${AZURE_TEMPLATE_SOURCE})

# SDK libraries should privately define _azure_BUILDING_SDK,
# tests should define _azure_BUILDING_TESTS,
# samples should have _azure_BUILDING_SAMPLES.
target_compile_definitions(azure-template PRIVATE _azure_BUILDING_SDK)

create_per_service_target_build(template azure-template)

target_include_directories(
  azure-template
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-template PUBLIC Azure::azure-core)

# make sure that users can consume the project as a library.
add_library(Azure::azure-template ALIAS azure-template)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(template azure-template azure-template-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
set_target_properties(azure-template PROPERTIES VERSION ${AZ_LIBRARY_VERSION})
generate_documentation(azure-template ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-template
    TEMPLATE
    "azure/template/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-template
  TEMPLATE
  "azure/template/rtti.hpp"
)

if(BUILD_TESTING)
    # tests
    add_subdirectory(test)
endif()
