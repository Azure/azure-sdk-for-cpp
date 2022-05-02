# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-storage-datamovement LANGUAGES CXX)

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

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(STORAGE_DATAMOVEMENT)
    GetFolderList(STORAGE_DATAMOVEMENT)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-storage-blobs-cpp "12.4.0" CONFIG QUIET)
  if(NOT azure-storage-blobs-cpp_FOUND)
    find_package(azure-storage-blobs-cpp "12.4.0" REQUIRED)
  endif()
endif()

set(
  AZURE_STORAGE_DATAMOVEMENT_HEADER
  inc/azure/storage/datamovement/blob_folder.hpp
  inc/azure/storage/datamovement/datamovement_options.hpp
  inc/azure/storage/datamovement/dll_import_export.hpp
  inc/azure/storage/datamovement/job_properties.hpp
  inc/azure/storage/datamovement/rtti.hpp
  inc/azure/storage/datamovement/scheduler.hpp
  inc/azure/storage/datamovement/storage_transfer_manager.hpp
  inc/azure/storage/datamovement/task.hpp
  inc/azure/storage/datamovement/tasks/upload_blob_from_file_task.hpp
  src/private/package_version.hpp
)

set(
  AZURE_STORAGE_DATAMOVEMENT_SOURCE
  src/scheduler.cpp
  src/storage_transfer_manager.cpp
  src/tasks/upload_blob_from_file_task.cpp
)

add_library(azure-storage-datamovement ${AZURE_STORAGE_DATAMOVEMENT_HEADER} ${AZURE_STORAGE_DATAMOVEMENT_SOURCE})
create_per_service_target_build(storage azure-storage-datamovement)

# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-datamovement ALIAS azure-storage-datamovement)

target_include_directories(
  azure-storage-datamovement
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-storage-datamovement PUBLIC Azure::azure-storage-blobs)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-storage-datamovement ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
  azure-storage-datamovement
    STORAGE_DATAMOVEMENT
    "azure/storage/datamovement/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-storage-datamovement
  STORAGE_DATAMOVEMENT
  "azure/storage/datamovement/rtti.hpp"
)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-datamovement azure-storage-test "tests?/*;samples?/*")

if(BUILD_TESTING)
  # add_subdirectory(test/ut)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
