# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-storage-files-datalake")
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_vcpkg)
include(az_version)
include(CodeCoverage)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-storage-blobs-cpp CONFIG)
  if(NOT azure-storage-blobs-cpp_FOUND)
    find_package(azure-storage-blobs-cpp REQUIRED)
  endif()
endif()

add_library(
  ${TARGET_NAME}
  src/datalake_directory_client.cpp
  src/datalake_file_client.cpp
  src/datalake_file_system_client.cpp
  src/datalake_path_client.cpp
  src/datalake_responses.cpp
  src/datalake_sas_builder.cpp
  src/datalake_service_client.cpp
  src/datalake_utilities.cpp
  )

# make sure that users can consume the project as a library.
add_library(Azure::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_include_directories(
  ${TARGET_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      ${azure-storage-blobs-cpp_INCLUDE_DIRS}
)

target_link_libraries(${TARGET_NAME} PUBLIC Azure::azure-storage-blobs)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/files/datalake/version.hpp")
generate_documentation(${TARGET_NAME} ${AZ_LIBRARY_VERSION})

az_vcpkg_export()

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-files-datalake azure-storage-test)

if(BUILD_TESTING)
  target_sources(
    azure-storage-test
      PRIVATE
        test/datalake_directory_client_test.cpp
        test/datalake_file_client_test.cpp
        test/datalake_file_system_client_test.cpp
        test/datalake_path_client_test.cpp
        test/datalake_sas_test.cpp
        test/datalake_service_client_test.cpp
  )

  target_link_libraries(azure-storage-test PRIVATE azure-storage-files-datalake)
endif()

if(BUILD_STORAGE_SAMPLES)
  target_sources(azure-storage-sample PRIVATE sample/datalake_getting_started.cpp)
  target_link_libraries(azure-storage-sample PRIVATE azure-storage-files-datalake)
endif()
