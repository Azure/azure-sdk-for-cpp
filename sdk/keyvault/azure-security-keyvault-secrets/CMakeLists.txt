# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-secrets LANGUAGES CXX)

# Compile Options
option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(SECRETS)
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
    GetFolderList(SECRETS)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder})
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.2.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.2.0" REQUIRED)
  endif()
endif()

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_HEADER
    inc/azure/keyvault/secrets/dll_import_export.hpp
    inc/azure/keyvault/secrets/keyvault_backup_secret.hpp
    inc/azure/keyvault/secrets/keyvault_deleted_secret.hpp
    inc/azure/keyvault/secrets/keyvault_operations.hpp
    inc/azure/keyvault/secrets/keyvault_options.hpp
    inc/azure/keyvault/secrets/keyvault_secret.hpp
    inc/azure/keyvault/secrets/keyvault_secret_paged_response.hpp
    inc/azure/keyvault/secrets/keyvault_secret_properties.hpp
    inc/azure/keyvault/secrets/rtti.hpp
    inc/azure/keyvault/secrets/secret_client.hpp
    inc/azure/keyvault/secrets.hpp
)

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE
    src/private/keyvault_protocol.hpp
    src/private/keyvault_secrets_common_request.hpp
    src/private/package_version.hpp
    src/private/secret_constants.hpp
    src/private/secret_serializers.hpp
    src/keyvault_operations.cpp
    src/keyvault_protocol.cpp
    src/keyvault_secret_paged_response.cpp
    src/keyvault_secret_properties.cpp
    src/keyvault_secrets_common_request.cpp
    src/secret_client.cpp
    src/secret_serializers.cpp
)

add_library(azure-security-keyvault-secrets ${AZURE_SECURITY_KEYVAULT_SECRETS_HEADER} ${AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE})
add_library(Azure::azure-security-keyvault-secrets ALIAS azure-security-keyvault-secrets)
create_per_service_target_build(keyvault azure-security-keyvault-secrets)

target_include_directories(
  azure-security-keyvault-secrets
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../../../sdk/core/inc>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/build/sdk/core/azure-core/inc>
      $<INSTALL_INTERFACE:include>
)

# Include shared source code
# NOTE: Use shared-code only within .cpp files. DO NEVER consume the shared-code from header files.
target_include_directories(
  azure-security-keyvault-secrets
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../azure-security-keyvault-shared/inc>
)

target_link_libraries(azure-security-keyvault-secrets PUBLIC Azure::azure-core)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-secrets azure-security-keyvault-secrets-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-security-keyvault-secrets ${AZ_LIBRARY_VERSION})
if(BUILD_TESTING)

  if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
  add_subdirectory(test/test-app)
endif()

if (BUILD_PERFORMANCE_TESTS)
  #add_subdirectory(test/perf)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(test/samples)
endif()

az_vcpkg_export(
    azure-security-keyvault-secrets
    SECURITY_KEYVAULT_SECRETS
    "azure/keyvault/secrets/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-security-keyvault-secrets
  SECURITY_KEYVAULT_SECRETS
  "azure/keyvault/secrets/rtti.hpp"
)

unset(FETCH_SOURCE_DEPS CACHE)
