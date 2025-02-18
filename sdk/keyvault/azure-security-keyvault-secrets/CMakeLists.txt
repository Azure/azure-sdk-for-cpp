# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

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

option(AZURE_TSP_KV_SECRETS_GEN "Generate KeyVault Secrets from TypeSpec" OFF)
message("KeyVault Secrets TSP Generation ${AZURE_TSP_KV_SECRETS_GEN}")

if(AZURE_TSP_KV_SECRETS_GEN)
    include(TSPCompile) 
    #typespec related repo information
    set(TSP_SERVICE_PATH "Security.KeyVault.Secrets")
    set(TSP_DESTINATION "keyvault_secrets")
    
    #codegen related repo information
    set(CODEGEN_SHA "a4371398f07ec577cbab98432986c0eda06942f9")
    set(CODEGEN_DESTINATION "typespec-cpp")
       
    #destination folders
    set(INCLUDE_DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/inc")
    set(SOURCE_DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}/src")

    #generate code from typespec
    GenerateCodeFromTypeSpec(${TSP_DESTINATION} ${TSP_SERVICE_PATH} ${CODEGEN_SHA} ${CODEGEN_DESTINATION})
    #update code files left this as a separate step as generating and moving result around
    #conceptually are two different steps
    UpdateCodeFilesFromGenerated(${CODEGEN_DESTINATION} ${TSP_SERVICE_PATH} ${INCLUDE_DESTINATION} ${SOURCE_DESTINATION})
endif()

if(FETCH_SOURCE_DEPS)
    GetFolderList(SECRETS)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder})
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_HEADER
    inc/azure/security/keyvault/secrets.hpp
    inc/azure/security/keyvault/secrets/dll_import_export.hpp
    inc/azure/security/keyvault/secrets/rtti.hpp
    inc/azure/security/keyvault/secrets/secrets_models.hpp
    inc/azure/security/keyvault/secrets/secret_client.hpp
    inc/azure/security/keyvault/secrets/secret_client_options.hpp
    inc/azure/security/keyvault/secrets/secret_client_paged_responses.hpp
    inc/azure/security/keyvault/secrets.hpp
)

set(
  AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE
    src/private/package_version.hpp
    src/deleted_secret_paged_response.cpp
    src/get_properties_of_secrets_versions_paged_response.cpp
    src/secrets.cpp
    src/secret_client.cpp
    src/secret_properties_paged_response.cpp
)

add_library(azure-security-keyvault-secrets ${AZURE_SECURITY_KEYVAULT_SECRETS_HEADER} ${AZURE_SECURITY_KEYVAULT_SECRETS_SOURCE})
add_library(Azure::azure-security-keyvault-secrets ALIAS azure-security-keyvault-secrets)
create_per_service_target_build(keyvault azure-security-keyvault-secrets)

target_include_directories(
  azure-security-keyvault-secrets
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
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

target_compile_definitions(azure-security-keyvault-secrets PRIVATE _azure_BUILDING_SDK)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-secrets azure-security-keyvault-secrets-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
set_target_properties(azure-security-keyvault-secrets PROPERTIES VERSION ${AZ_LIBRARY_VERSION})
generate_documentation(azure-security-keyvault-secrets ${AZ_LIBRARY_VERSION})
if(BUILD_TESTING)

  if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
#  add_subdirectory(test/ut)
#  add_subdirectory(test/test-app)
endif()

if (BUILD_PERFORMANCE_TESTS)
 # add_subdirectory(test/perf)
endif()

if(BUILD_SAMPLES)
 # add_subdirectory(samples)
endif()

az_vcpkg_export(
    azure-security-keyvault-secrets
    SECURITY_KEYVAULT_SECRETS
    "azure/security/keyvault/secrets/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-security-keyvault-secrets
  SECURITY_KEYVAULT_SECRETS
  "azure/security/keyvault/secrets/rtti.hpp"
)

unset(FETCH_SOURCE_DEPS CACHE)
