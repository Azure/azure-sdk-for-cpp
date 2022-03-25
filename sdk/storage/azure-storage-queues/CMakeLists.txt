# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-queues LANGUAGES CXX)

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
    SetCompileOptions(STORAGE_QUEUES)
    GetFolderList(STORAGE_QUEUES)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-storage-common-cpp "12.2.3" CONFIG QUIET)
  if(NOT azure-storage-common-cpp_FOUND)
    find_package(azure-storage-common-cpp "12.2.3" REQUIRED)
  endif()
endif()

set(
  AZURE_STORAGE_QUEUE_HEADER
    inc/azure/storage/queues.hpp
    inc/azure/storage/queues/dll_import_export.hpp
    inc/azure/storage/queues/queue_client.hpp
    inc/azure/storage/queues/queue_options.hpp
    inc/azure/storage/queues/queue_responses.hpp
    inc/azure/storage/queues/queue_sas_builder.hpp
    inc/azure/storage/queues/queue_service_client.hpp
    inc/azure/storage/queues/rest_client.hpp
)

set(
  AZURE_STORAGE_QUEUE_SOURCE
    src/private/package_version.hpp
    src/queue_client.cpp
    src/queue_options.cpp
    src/queue_responses.cpp
    src/queue_sas_builder.cpp
    src/queue_service_client.cpp
    src/rest_client.cpp
)

add_library(azure-storage-queues ${AZURE_STORAGE_QUEUE_HEADER} ${AZURE_STORAGE_QUEUE_SOURCE})
create_per_service_target_build(storage azure-storage-queues)
# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-queues ALIAS azure-storage-queues)

target_include_directories(
  azure-storage-queues
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-storage-queues PUBLIC Azure::azure-storage-common)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-storage-queues ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-storage-queues
    STORAGE_QUEUES
    "azure/storage/queues/dll_import_export.hpp"
  )
  
az_rtti_setup(
  azure-storage-queues
  STORAGE_QUEUES
  "azure/storage/queues/rtti.hpp"
)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-queues azure-storage-test "tests?/*;samples?/*")

if(BUILD_TESTING)
  add_subdirectory(test/ut)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
