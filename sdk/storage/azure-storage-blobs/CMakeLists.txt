# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-blobs LANGUAGES CXX)

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
  find_package(azure-storage-common-cpp CONFIG QUIET)
  if(NOT azure-storage-common-cpp_FOUND)
    find_package(azure-storage-common-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_STORAGE_BLOB_HEADER
    inc/azure/storage/blobs/protocol/blob_rest_client.hpp
    inc/azure/storage/blobs/append_blob_client.hpp
    inc/azure/storage/blobs/blob_client.hpp
    inc/azure/storage/blobs/blob_container_client.hpp
    inc/azure/storage/blobs/blob_lease_client.hpp
    inc/azure/storage/blobs/blob_options.hpp
    inc/azure/storage/blobs/blob_responses.hpp
    inc/azure/storage/blobs/blob_sas_builder.hpp
    inc/azure/storage/blobs/blob_service_client.hpp
    inc/azure/storage/blobs/block_blob_client.hpp
    inc/azure/storage/blobs/dll_import_export.hpp
    inc/azure/storage/blobs/page_blob_client.hpp
    inc/azure/storage/blobs/version.hpp
    inc/azure/storage/blobs.hpp
)

set(
  AZURE_STORAGE_BLOB_SOURCE
    src/append_blob_client.cpp
    src/blob_client.cpp
    src/blob_container_client.cpp
    src/blob_lease_client.cpp
    src/blob_responses.cpp
    src/blob_rest_client.cpp
    src/blob_sas_builder.cpp
    src/blob_service_client.cpp
    src/block_blob_client.cpp
    src/page_blob_client.cpp
)

add_library(azure-storage-blobs ${AZURE_STORAGE_BLOB_HEADER} ${AZURE_STORAGE_BLOB_SOURCE})

# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-blobs ALIAS azure-storage-blobs)

target_include_directories(
  azure-storage-blobs
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-storage-blobs PUBLIC Azure::azure-storage-common)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/blobs/version.hpp")
generate_documentation(azure-storage-blobs ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-storage-blobs
    STORAGE_BLOBS
    "azure/storage/blobs/dll_import_export.hpp"
  )

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-blobs azure-storage-test)

if(BUILD_TESTING)
  target_sources(
    azure-storage-test
      PRIVATE
        test/append_blob_client_test.cpp
        test/append_blob_client_test.hpp
        test/blob_container_client_test.cpp
        test/blob_container_client_test.hpp
        test/blob_sas_test.cpp
        test/blob_service_client_test.cpp
        test/block_blob_client_test.cpp
        test/block_blob_client_test.hpp
        test/page_blob_client_test.cpp
        test/page_blob_client_test.hpp
        test/storage_retry_policy_test.cpp
  )

  target_link_libraries(azure-storage-test PRIVATE azure-storage-blobs)
endif()

if(BUILD_STORAGE_SAMPLES)
  target_sources(
    azure-storage-sample
      PRIVATE
        sample/blob_getting_started.cpp
  )

  target_link_libraries(azure-storage-sample PRIVATE azure-storage-blobs)
endif()

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(test/performance)
endif()
