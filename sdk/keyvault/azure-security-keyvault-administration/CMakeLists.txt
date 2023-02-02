# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-administration LANGUAGES CXX)

# Compile Options
option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(ADMINISTRATION)
endif()

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)
# Add create_map_file function
include(CreateMapFile)

if(FETCH_SOURCE_DEPS)
    GetFolderList(ADMINISTRATION)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder})
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.8.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.8.0" REQUIRED)
  endif()
endif()

set(
  AZURE_SECURITY_KEYVAULT_ADMINISTRATION_HEADER
    inc/azure/keyvault/administration.hpp
    inc/azure/keyvault/administration/dll_import_export.hpp
    inc/azure/keyvault/administration/rest_client_models.hpp
    inc/azure/keyvault/administration/rtti.hpp
    inc/azure/keyvault/administration/settings_client.hpp
    inc/azure/keyvault/administration/settings_client_options.hpp
    src/private/keyvault_settings_common_request.hpp
    src/private/administration_constants.hpp
)

set(
  AZURE_SECURITY_KEYVAULT_ADMINISTRATION_SOURCE
    src/keyvault_settings_common_request.cpp
    src/settings_client.cpp
)

add_library(azure-security-keyvault-administration ${AZURE_SECURITY_KEYVAULT_ADMINISTRATION_HEADER} ${AZURE_SECURITY_KEYVAULT_ADMINISTRATION_SOURCE})
add_library(Azure::azure-security-keyvault-administration ALIAS azure-security-keyvault-administration)
create_per_service_target_build(keyvault azure-security-keyvault-administration)

target_include_directories(
  azure-security-keyvault-administration
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

# Include shared source code
# NOTE: Use shared-code only within .cpp files. DO NEVER consume the shared-code from header files.
target_include_directories(
  azure-security-keyvault-administration
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../azure-security-keyvault-shared/inc>
)

target_link_libraries(azure-security-keyvault-administration PUBLIC Azure::azure-core)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-administration azure-security-keyvault-administration-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-security-keyvault-administration ${AZ_LIBRARY_VERSION})
if(BUILD_TESTING)

  if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
endif()

if (BUILD_PERFORMANCE_TESTS)
  #add_subdirectory(test/perf)
endif()

if(BUILD_SAMPLES_HSM)
  add_subdirectory(test/samples)
endif()

az_vcpkg_export(
    azure-security-keyvault-administration
    SECURITY_KEYVAULT_ADMINISTRATION
    azure/keyvault/administration/dll_import_export.hpp
  )

az_rtti_setup(
  azure-security-keyvault-administration
  SECURITY_KEYVAULT_ADMINISTRATION
  azure/keyvault/administration/rtti.hpp
)

unset(FETCH_SOURCE_DEPS CACHE)
