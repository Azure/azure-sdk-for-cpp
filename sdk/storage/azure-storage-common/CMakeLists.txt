# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-common LANGUAGES CXX)

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
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

find_package(Threads REQUIRED)
find_package(LibXml2 REQUIRED)

set(
  AZURE_STORAGE_COMMON_HEADER
    inc/azure/storage/common/access_conditions.hpp
    inc/azure/storage/common/account_sas_builder.hpp
    inc/azure/storage/common/concurrent_transfer.hpp
    inc/azure/storage/common/constants.hpp
    inc/azure/storage/common/crypt.hpp
    inc/azure/storage/common/file_io.hpp
    inc/azure/storage/common/reliable_stream.hpp
    inc/azure/storage/common/shared_key_policy.hpp
    inc/azure/storage/common/storage_common.hpp
    inc/azure/storage/common/storage_credential.hpp
    inc/azure/storage/common/storage_exception.hpp
    inc/azure/storage/common/storage_per_retry_policy.hpp
    inc/azure/storage/common/storage_retry_policy.hpp
    inc/azure/storage/common/version.hpp
    inc/azure/storage/common/xml_wrapper.hpp
)

set(
  AZURE_STORAGE_COMMON_SOURCE
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

add_library(azure-storage-common ${AZURE_STORAGE_COMMON_HEADER} ${AZURE_STORAGE_COMMON_SOURCE})

# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-common ALIAS azure-storage-common)

target_include_directories(
  azure-storage-common
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-storage-common PUBLIC Azure::azure-core)
target_include_directories(azure-storage-common PRIVATE ${LIBXML2_INCLUDE_DIRS})
target_link_libraries(azure-storage-common PRIVATE ${LIBXML2_LIBRARIES})

if(MSVC)
    target_link_libraries(azure-storage-common PRIVATE bcrypt)
    # C28020 and C28204 are introduced by nlohmann/json
    target_compile_options(azure-storage-common PUBLIC /wd28204 /wd28020)
else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-storage-common PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/storage/common/version.hpp")
generate_documentation(azure-storage-common ${AZ_LIBRARY_VERSION})

az_vcpkg_export(azure-storage-common)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
# excluding json from coverage report
create_code_coverage(storage azure-storage-common azure-storage-test "inc/azure/storage/common/json*")

if(BUILD_TESTING)
  target_sources(
    azure-storage-test
      PRIVATE
        test/bearer_token_test.cpp
        test/crypt_functions_test.cpp
        test/metadata_test.cpp
        test/storage_credential_test.cpp
        test/test_base.cpp
        test/test_base.hpp
  )

  if (MSVC)
    target_compile_options(azure-storage-test PUBLIC /wd6326 /wd26495 /wd26812)
  endif()

  target_link_libraries(azure-storage-test PRIVATE azure-identity)
  target_include_directories(azure-storage-test PRIVATE test)
endif()

if(BUILD_STORAGE_SAMPLES)
  target_sources(
    azure-storage-sample
      PRIVATE
        sample/main.cpp
        sample/samples_common.hpp
  )

  target_include_directories(azure-storage-sample PRIVATE sample)
endif()
