# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-storage-files-shares")
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
  find_package(azure-storage-common-cpp CONFIG)
  if(NOT azure-storage-common-cpp_FOUND)
    find_package(azure-storage-common-cpp REQUIRED)
  endif()
endif()

add_library(
  ${TARGET_NAME}
  src/share_client.cpp
  src/share_directory_client.cpp
  src/share_file_client.cpp
  src/share_sas_builder.cpp
  src/share_service_client.cpp
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

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/files/shares/version.hpp")
generate_documentation(${TARGET_NAME} ${AZ_LIBRARY_VERSION})

az_vcpkg_export()

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(storage azure-storage-files-shares azure-storage-test)

if(BUILD_TESTING)
  target_sources(
    azure-storage-test
      PRIVATE
        test/share_client_test.cpp
        test/share_directory_client_test.cpp
        test/share_file_client_test.cpp
        test/share_sas_test.cpp
        test/share_service_client_test.cpp
  )

  target_link_libraries(azure-storage-test PRIVATE azure-storage-files-shares)
endif()

if(BUILD_STORAGE_SAMPLES)
  target_sources(azure-storage-sample PRIVATE sample/file_share_getting_started.cpp)
  target_link_libraries(azure-storage-sample PRIVATE azure-storage-files-shares)
endif()
