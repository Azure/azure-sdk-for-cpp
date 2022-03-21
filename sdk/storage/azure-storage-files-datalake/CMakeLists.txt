# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-files-datalake LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(FETCH_SOURCE_DEPS "build source dependencies" OFF)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(AzureVcpkg)
include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)

az_vcpkg_integrate()

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(STORAGE_DATALAKE)
    GetFolderList(STORAGE_DATALAKE)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-storage-blobs-cpp "12.3.0" CONFIG QUIET)
  if(NOT azure-storage-blobs-cpp_FOUND)
    find_package(azure-storage-blobs-cpp "12.3.0" REQUIRED)
  endif()
endif()

set(
  AZURE_STORAGE_FILES_DATALAKE_HEADER
    inc/azure/storage/files/datalake.hpp
    inc/azure/storage/files/datalake/datalake_directory_client.hpp
    inc/azure/storage/files/datalake/datalake_file_client.hpp
    inc/azure/storage/files/datalake/datalake_file_system_client.hpp
    inc/azure/storage/files/datalake/datalake_lease_client.hpp
    inc/azure/storage/files/datalake/datalake_options.hpp
    inc/azure/storage/files/datalake/datalake_path_client.hpp
    inc/azure/storage/files/datalake/datalake_responses.hpp
    inc/azure/storage/files/datalake/datalake_sas_builder.hpp
    inc/azure/storage/files/datalake/datalake_service_client.hpp
    inc/azure/storage/files/datalake/dll_import_export.hpp
    inc/azure/storage/files/datalake/rest_client.hpp
)

set(
  AZURE_STORAGE_FILES_DATALAKE_SOURCE
    src/private/datalake_constants.hpp
    src/private/datalake_utilities.hpp
    src/private/package_version.hpp
    src/datalake_directory_client.cpp
    src/datalake_file_client.cpp
    src/datalake_file_system_client.cpp
    src/datalake_lease_client.cpp
    src/datalake_path_client.cpp
    src/datalake_responses.cpp
    src/datalake_sas_builder.cpp
    src/datalake_service_client.cpp
    src/datalake_utilities.cpp
    src/rest_client.cpp
)

add_library(azure-storage-files-datalake ${AZURE_STORAGE_FILES_DATALAKE_HEADER} ${AZURE_STORAGE_FILES_DATALAKE_SOURCE})

# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-files-datalake ALIAS azure-storage-files-datalake)
create_per_service_target_build(storage azure-storage-files-datalake)

target_include_directories(
  azure-storage-files-datalake
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-storage-files-datalake PUBLIC Azure::azure-storage-blobs)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-storage-files-datalake ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-storage-files-datalake
    STORAGE_FILES_DATALAKE
    "azure/storage/files/datalake/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-storage-files-datalake
  STORAGE_FILES_DATALAKE
  "azure/storage/files/datalake/rtti.hpp"
)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-files-datalake azure-storage-test "tests?/*;samples?/*")

if(BUILD_TESTING)
  add_subdirectory(test/ut)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
