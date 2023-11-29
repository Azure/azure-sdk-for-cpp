# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-storage-tables LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)
include(CreateMapFile)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(STORAGE_TABLES)
    GetFolderList(STORAGE_TABLES)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-storage-common-cpp CONFIG QUIET)
  if(NOT azure-storage-common-cpp_FOUND)
    find_package(azure-storage-common-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_STORAGE_TABLES_HEADER
    inc/azure/storage/tables/dll_import_export.hpp
    inc/azure/storage/tables/rest_client.hpp
    inc/azure/storage/tables/rtti.hpp
    inc/azure/storage/tables/models.hpp
    inc/azure/storage/tables/transactions.hpp
    inc/azure/storage/tables/serializers.hpp
)

set(
  AZURE_STORAGE_TABLES_SOURCE
    src/private/package_version.hpp
    src/rest_client.cpp
    src/transactions.cpp
)

add_library(azure-storage-tables ${AZURE_STORAGE_TABLES_HEADER} ${AZURE_STORAGE_TABLES_SOURCE})
create_per_service_target_build(storage azure-storage-tables)
# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-tables ALIAS azure-storage-tables)

target_include_directories(
  azure-storage-tables
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-storage-tables PUBLIC Azure::azure-storage-common)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-storage-tables ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-storage-tables
    STORAGE_TABLES
    azure/storage/tables/dll_import_export.hpp
  )
  
az_rtti_setup(
  azure-storage-tables
  STORAGE_TABLES
  azure/storage/tables/rtti.hpp
)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-tables azure-storage-test "tests?/*;samples?/*")

if(BUILD_TESTING)
  add_subdirectory(test/ut)
endif()

#if(BUILD_SAMPLES)
#  add_subdirectory(samples)
#endif()
