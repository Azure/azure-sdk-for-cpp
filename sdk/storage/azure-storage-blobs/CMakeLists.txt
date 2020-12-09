# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-blobs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_version)
include(CodeCoverage)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

set(
  AZURE_STORAGE_BLOB_HEADER
    src/append_blob_client.cpp
    src/blob_batch_client.cpp
    src/blob_client.cpp
    src/blob_container_client.cpp
    src/blob_sas_builder.cpp
    src/blob_service_client.cpp
    src/block_blob_client.cpp
    src/page_blob_client.cpp
)

set(
  AZURE_STORAGE_BLOB_SOURCE
    inc/azure/storage/blobs/protocol/blob_rest_client.hpp
    inc/azure/storage/blobs/append_blob_client.hpp
    inc/azure/storage/blobs/blob_batch_client.hpp
    inc/azure/storage/blobs/blob_client.hpp
    inc/azure/storage/blobs/blob_container_client.hpp
    inc/azure/storage/blobs/blob_options.hpp
    inc/azure/storage/blobs/blob_responses.hpp
    inc/azure/storage/blobs/blob_sas_builder.hpp
    inc/azure/storage/blobs/blob_service_client.hpp
    inc/azure/storage/blobs/block_blob_client.hpp
    inc/azure/storage/blobs/page_blob_client.hpp
    inc/azure/storage/blobs/version.hpp
    inc/azure/storage/blobs.hpp
)

add_library(azure-storage-blobs ${AZURE_STORAGE_BLOB_HEADER} ${AZURE_STORAGE_BLOB_SOURCE})

# make sure that users can consume the project as a library.
add_library(azure::storage::blobs ALIAS azure-storage-blobs)

target_include_directories(azure-storage-blobs PUBLIC inc)
target_link_libraries(azure-storage-blobs azure::storage::common)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/blobs/version.hpp")
generate_documentation(azure-storage-blobs ${AZ_LIBRARY_VERSION})

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-blobs azure-storage-test)

if(BUILD_TESTING)
  target_sources(
    azure-storage-test
      PRIVATE
        test/append_blob_client_test.cpp
        test/append_blob_client_test.hpp
        test/blob_batch_client_test.cpp
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
