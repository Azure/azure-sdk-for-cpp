# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
set(TARGET_NAME "azure-storage-common")
project(${TARGET_NAME} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_vcpkg)
include(az_version)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

az_vcpkg_integrate()

if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)

add_library(
  ${TARGET_NAME}
  src/account_sas_builder.cpp
  src/crypt.cpp
  src/file_io.cpp
  src/reliable_stream.cpp
  src/shared_key_policy.cpp
  src/storage_common.cpp
  src/storage_credential.cpp
  src/storage_exception.cpp
  src/storage_per_retry_policy.cpp
  src/storage_retry_policy.cpp
  src/xml_wrapper.cpp
  )

# make sure that users can consume the project as a library.
add_library(Azure::${TARGET_NAME} ALIAS ${TARGET_NAME})

target_include_directories(
  ${TARGET_NAME}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      ${azure-core-cpp_INCLUDE_DIRS}
  )

target_include_directories(${TARGET_NAME} INTERFACE ${LIBXML2_INCLUDE_DIRS})
target_link_libraries(${TARGET_NAME} PUBLIC Azure::azure-core)
target_link_libraries(${TARGET_NAME} INTERFACE Threads::Threads ${LIBXML2_LIBRARIES})

if(MSVC)
    target_link_libraries(${TARGET_NAME} INTERFACE bcrypt)
    # C28020 and C28204 are introduced by nlohmann/json
    target_compile_options(${TARGET_NAME} PUBLIC /wd28204 /wd28020)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(${TARGET_NAME} INTERFACE OpenSSL::SSL OpenSSL::Crypto)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/common/version.hpp")
generate_documentation(${TARGET_NAME} ${AZ_LIBRARY_VERSION})

az_vcpkg_export()
