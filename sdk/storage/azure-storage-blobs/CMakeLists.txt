# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-storage-blobs")
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_vcpkg)
include(az_version)
include(CodeCoverage)
include(DefinePlatform)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-storage-common-cpp CONFIG)
  if(NOT azure-storage-common-cpp_FOUND)
    find_package(azure-storage-common-cpp REQUIRED)
  endif()
endif()

add_library(
  ${TARGET_NAME}
  src/append_blob_client.cpp
  src/blob_batch_client.cpp
  src/blob_client.cpp
  src/blob_container_client.cpp
  src/blob_sas_builder.cpp
  src/blob_service_client.cpp
  src/block_blob_client.cpp
  src/page_blob_client.cpp
  )

# make sure that users can consume the project as a library.
add_library(Azure::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_include_directories(
  ${TARGET_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      ${azure-storage-common-cpp_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME} PUBLIC Azure::azure-storage-common)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/blobs/version.hpp")
generate_documentation(${TARGET_NAME} ${AZ_LIBRARY_VERSION})

az_vcpkg_export()

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-blobs azure-storage-test)

if(BUILD_TESTING)
  target_sources(
    azure-storage-test
      PRIVATE
        test/append_blob_client_test.cpp
        test/blob_batch_client_test.cpp
        test/blob_container_client_test.cpp
        test/blob_sas_test.cpp
        test/blob_service_client_test.cpp
        test/block_blob_client_test.cpp
        test/page_blob_client_test.cpp
        test/storage_retry_policy_test.cpp
  )

  target_link_libraries(azure-storage-test PRIVATE azure-storage-blobs)
endif()

if(BUILD_STORAGE_SAMPLES)
  target_sources(azure-storage-sample PRIVATE sample/blob_getting_started.cpp)
  target_link_libraries(azure-storage-sample PRIVATE azure-storage-blobs)
endif()
