# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-storage-common LANGUAGES CXX)

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
    SetCompileOptions(STORAGE_COMMON)
    GetFolderList(STORAGE_COMMON)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.3.1" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.3.1" REQUIRED)
  endif()
endif()

find_package(Threads REQUIRED)

set(
  AZURE_STORAGE_COMMON_HEADER
    inc/azure/storage/common/access_conditions.hpp
    inc/azure/storage/common/account_sas_builder.hpp
    inc/azure/storage/common/crypt.hpp
    inc/azure/storage/common/dll_import_export.hpp
    inc/azure/storage/common/internal/concurrent_transfer.hpp
    inc/azure/storage/common/internal/constants.hpp
    inc/azure/storage/common/internal/file_io.hpp
    inc/azure/storage/common/internal/reliable_stream.hpp
    inc/azure/storage/common/internal/shared_key_policy.hpp
    inc/azure/storage/common/internal/storage_per_retry_policy.hpp
    inc/azure/storage/common/internal/storage_service_version_policy.hpp
    inc/azure/storage/common/internal/storage_switch_to_secondary_policy.hpp
    inc/azure/storage/common/internal/xml_wrapper.hpp
    inc/azure/storage/common/storage_common.hpp
    inc/azure/storage/common/storage_credential.hpp
    inc/azure/storage/common/storage_exception.hpp
)

set(
  AZURE_STORAGE_COMMON_SOURCE
    src/private/package_version.hpp
    src/account_sas_builder.cpp
    src/crypt.cpp
    src/file_io.cpp
    src/reliable_stream.cpp
    src/shared_key_policy.cpp
    src/storage_common.cpp
    src/storage_credential.cpp
    src/storage_exception.cpp
    src/storage_per_retry_policy.cpp
    src/storage_switch_to_secondary_policy.cpp
    src/xml_wrapper.cpp
)

add_library(azure-storage-common ${AZURE_STORAGE_COMMON_HEADER} ${AZURE_STORAGE_COMMON_SOURCE})
create_per_service_target_build(azure-storage-common storage)

# make sure that users can consume the project as a library.
add_library(Azure::azure-storage-common ALIAS azure-storage-common)

target_include_directories(
  azure-storage-common
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-storage-common PUBLIC Azure::azure-core)

if(WIN32)
    target_link_libraries(azure-storage-common PRIVATE bcrypt webservices)
    # C28020 and C28204 are introduced by nlohmann/json
    target_compile_options(azure-storage-common PUBLIC /wd28204 /wd28020)
else()
    find_package(LibXml2 REQUIRED)
    target_include_directories(azure-storage-common SYSTEM PRIVATE ${LIBXML2_INCLUDE_DIRS})
    target_link_libraries(azure-storage-common PRIVATE ${LIBXML2_LIBRARIES})
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-storage-common PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-storage-common ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-storage-common
    STORAGE_COMMON
    "azure/storage/common/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-storage-common
  STORAGE_COMMON
  "azure/storage/common/rtti.hpp"
)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
# excluding json from coverage report
create_code_coverage(storage azure-storage-common azure-storage-test "tests?/*;samples?/*")

if(BUILD_TESTING)
  add_subdirectory(test/ut)
endif()

unset(FETCH_SOURCE_DEPS CACHE)
