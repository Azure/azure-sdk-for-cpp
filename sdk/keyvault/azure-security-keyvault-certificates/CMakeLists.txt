# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-certificates LANGUAGES CXX)


# Compile Options
option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(CERTIFICATES)
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
    GetFolderList(CERTIFICATES)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.5.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.5.0" REQUIRED)
  endif()
endif()

set(
  AZURE_KEYVAULT_CERTIFICATES_HEADER
    inc/azure/keyvault/certificates/certificate_client.hpp
    inc/azure/keyvault/certificates/certificate_client_models.hpp
    inc/azure/keyvault/certificates/certificate_client_options.hpp
    inc/azure/keyvault/certificates/certificate_client_operations.hpp
)

set(
  AZURE_KEYVAULT_CERTIFICATES_SOURCE
    src/certificate_client.cpp
    src/certificate_serializers.cpp
    src/keyvault_certificates_common_request.cpp
    src/certificate_client_operations.cpp
    src/certificate_client_paged_response.cpp
)

add_library(azure-security-keyvault-certificates
  ${AZURE_KEYVAULT_CERTIFICATES_HEADER} ${AZURE_KEYVAULT_CERTIFICATES_SOURCE}
)
add_library(Azure::azure-security-keyvault-certificates ALIAS azure-security-keyvault-certificates)
create_per_service_target_build(keyvault azure-security-keyvault-certificates)

target_include_directories(
  azure-security-keyvault-certificates
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/../../../sdk/core/inc>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/build/sdk/core/azure-core/inc>
      $<INSTALL_INTERFACE:include>
)
message($<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc)
# Include shared source code
# NOTE: Use shared-code only within .cpp files. DO NEVER consume the shared-code from header files.
target_include_directories(
  azure-security-keyvault-certificates
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../azure-security-keyvault-shared/inc>
)

target_link_libraries(azure-security-keyvault-certificates PUBLIC Azure::azure-core)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-certificates azure-security-keyvault-certificates-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-security-keyvault-certificates ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)
  # define a symbol that enables some test hooks in code
  add_compile_definitions(TESTING_BUILD)

  if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)  
endif()


# if (BUILD_PERFORMANCE_TESTS)
#   add_subdirectory(test/perf)
# endif()

if(BUILD_SAMPLES)
   add_subdirectory(test/samples)
endif()

az_vcpkg_export(
    azure-security-keyvault-certificates
    SECURITY_KEYVAULT_CERTIFICATES
    "azure/keyvault/certificates/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-security-keyvault-certificates
  SECURITY_KEYVAULT_CERTIFICATES
  "azure/keyvault/certificates/rtti.hpp"
)

unset(FETCH_SOURCE_DEPS CACHE)

