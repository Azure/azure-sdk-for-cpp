# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# Configure CMake project.
cmake_minimum_required (VERSION 3.13)
project(azure-storage-blobs-perf LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
include(AzureVcpkg)
az_vcpkg_integrate()

if(BUILD_TRANSPORT_CURL)
  set(DOWNLOAD_WITH_LIBCURL inc/azure/storage/blobs/test/download_blob_transport_only.hpp)
endif()

set(
  AZURE_STORAGE_BLOBS_PERF_TEST_HEADER
  inc/azure/storage/blobs/test/blob_base_test.hpp
  inc/azure/storage/blobs/test/download_blob_from_sas.hpp
  inc/azure/storage/blobs/test/download_blob_pipeline_only.hpp
  inc/azure/storage/blobs/test/download_blob_test.hpp
  ${DOWNLOAD_WITH_LIBCURL}
  inc/azure/storage/blobs/test/list_blob_test.hpp
  inc/azure/storage/blobs/test/upload_blob_test.hpp
)

set(
  AZURE_STORAGE_BLOBS_PERF_TEST_SOURCE
    src/azure_storage_blobs_perf_test.cpp
)

# Name the binary to be created.
add_executable (
  azure-storage-blobs-perf
     ${AZURE_STORAGE_BLOBS_PERF_TEST_HEADER} ${AZURE_STORAGE_BLOBS_PERF_TEST_SOURCE}
)

create_per_service_target_build(storage azure-storage-blobs-perf)

# Include the headers from the project.
#target_include_directories(
#  azure-storage-blobs-perf
#    PUBLIC
#    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
#)

if (MSVC)
    # allow msvc to use getenv()
    target_compile_options(azure-storage-blobs-perf PUBLIC /wd4996)
endif()
include(PerfTest)
SETPERFDEPS(azure-storage-blobs-cpp VCPKG_STORAGE_BLOB_VERSION)

# link the `azure-perf` lib together with any other library which will be used for the tests. 
target_link_libraries(azure-storage-blobs-perf PRIVATE Azure::azure-storage-blobs azure-perf)
# Make sure the project will appear in the test folder for Visual Studio CMake view
set_target_properties(azure-storage-blobs-perf PROPERTIES FOLDER "Tests/Storage")
