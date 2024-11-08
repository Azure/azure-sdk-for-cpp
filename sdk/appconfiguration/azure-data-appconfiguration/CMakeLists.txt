# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-data-appconfiguration LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

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
  AZURE_DATA_APPCONFIGURATION_HEADER
    inc/azure/data/appconfiguration.hpp
    inc/azure/data/appconfiguration/configuration_client.hpp
    inc/azure/data/appconfiguration/configuration_client_models.hpp
    inc/azure/data/appconfiguration/configuration_client_options.hpp
    inc/azure/data/appconfiguration/dll_import_export.hpp
    inc/azure/data/appconfiguration/rtti.hpp
)

set(
  AZURE_DATA_APPCONFIGURATION_SOURCE
    src/private/package_version.hpp
    src/configuration_client.cpp
)

add_library(azure-data-appconfiguration ${AZURE_DATA_APPCONFIGURATION_HEADER} ${AZURE_DATA_APPCONFIGURATION_SOURCE})

target_compile_definitions(azure-data-appconfiguration PRIVATE _azure_BUILDING_SDK)

create_per_service_target_build(appconfiguration azure-data-appconfiguration)

target_include_directories(
  azure-data-appconfiguration
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-data-appconfiguration PUBLIC Azure::azure-core)

# make sure that users can consume the project as a library.
add_library(Azure::azure-data-appconfiguration ALIAS azure-data-appconfiguration)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(appconfiguration azure-data-appconfiguration azure-data-appconfiguration-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
set_target_properties(azure-data-appconfiguration PROPERTIES VERSION ${AZ_LIBRARY_VERSION})
generate_documentation(azure-data-appconfiguration ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-data-appconfiguration
    DATA_APPCONFIGURATION
    "azure/data/appconfiguration/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-data-appconfiguration
  DATA_APPCONFIGURATION
  "azure/data/appconfiguration/rtti.hpp"
)

if(BUILD_TESTING)
    if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
      include(AddGoogleTest)
      enable_testing ()
    endif()

    # tests
    add_subdirectory(test/ut)
endif()
