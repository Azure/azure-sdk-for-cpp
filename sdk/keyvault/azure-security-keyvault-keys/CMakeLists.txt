# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# setting CMAKE_TOOLCHAIN_FILE must happen before creating the project
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-keys LANGUAGES CXX)

# Compile Options
option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(KEYS)
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
    GetFolderList(KEYS)
    foreach(oneFolder IN LISTS BUILD_FOLDERS)
        message("add folder ${oneFolder}")
        add_subdirectory(${oneFolder} EXCLUDE_FROM_ALL)
    endforeach()
elseif(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp REQUIRED)
  endif()
endif()

option(AZURE_TSP_KV_KEYS_GEN "Generate KeyVault Keys from TypeSpec" OFF)
message("KeyVault Secrets TSP Generation ${AZURE_TSP_KV_KEYS_GEN}")

if(AZURE_TSP_KV_KEYS_GEN)
    include(TSPCompile) 
    #typespec related repo information
    set(TSP_SERVICE_PATH "Security.KeyVault.Keys")
    set(TSP_DESTINATION "keyvault_keys")
    
    #codegen related repo information
    set(CODEGEN_SHA "83758baff29b89a8a7bd28fa26a692225f4f208f")
    set(CODEGEN_DESTINATION "typespec-cpp")
       
    #destination folders
    set(GEN_FILES_DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")
    #generate code from typespec
    GenerateCodeFromTypeSpec(${TSP_DESTINATION} ${TSP_SERVICE_PATH} ${CODEGEN_SHA} ${CODEGEN_DESTINATION} ${GEN_FILES_DESTINATION} OFF)
endif()
set(
  AZURE_KEYVAULT_KEYS_HEADER
    inc/azure/keyvault/keys.hpp
    inc/azure/keyvault/keys/cryptography/cryptography_client.hpp
    inc/azure/keyvault/keys/cryptography/cryptography_client_models.hpp
    inc/azure/keyvault/keys/cryptography/cryptography_client_options.hpp
    inc/azure/keyvault/keys/dll_import_export.hpp
    inc/azure/keyvault/keys/key_client.hpp
    inc/azure/keyvault/keys/key_client_models.hpp
    inc/azure/keyvault/keys/key_client_options.hpp
    inc/azure/keyvault/keys/rtti.hpp
    inc/azure/keyvault/keyvault_keys.hpp
)

set(
  AZURE_KEYVAULT_KEYS_SOURCE
    src/cryptography/cryptography_client.cpp
    src/cryptography/decrypt_parameters.cpp
    src/cryptography/decrypt_result.cpp
    src/cryptography/encrypt_parameters.cpp
    src/cryptography/encrypt_result.cpp
    src/cryptography/encryption_algorithm.cpp
    src/cryptography/key_sign_parameters.cpp
    src/cryptography/key_verify_parameters.cpp
    src/cryptography/key_wrap_algorithm.cpp
    src/cryptography/key_wrap_parameters.cpp
    src/cryptography/sign_result.cpp
    src/cryptography/signature_algorithm.cpp
    src/cryptography/unwrap_result.cpp
    src/cryptography/verify_result.cpp
    src/cryptography/wrap_result.cpp
    src/delete_key_operation.cpp
    src/key_client.cpp
    src/key_client_models.cpp
    src/key_client_options.cpp
    src/key_client_paged_responses.cpp
    src/key_curve_name.cpp
    src/key_encryption_algorithm.cpp
    src/key_operation.cpp
    src/key_type.cpp
    src/keyvault_protocol.cpp
    src/generated/get_deleted_keys_paged_response.cpp
    src/generated/get_keys_paged_response.cpp
    src/generated/get_key_versions_paged_response.cpp
    src/generated/keys.cpp
    src/generated/keys_models.hpp
    src/generated/key_vault_client.cpp
    src/generated/key_vault_client.hpp
    src/generated/key_vault_client_options.hpp
    src/generated/key_vault_client_paged_responses.hpp
    src/private/cryptography_internal_access.hpp
    src/private/cryptography_serializers.hpp
    src/private/key_constants.hpp
    src/private/key_serializers.hpp
    src/private/key_sign_parameters.hpp
    src/private/key_verify_parameters.hpp
    src/private/key_wrap_parameters.hpp
    src/private/keyvault_constants.hpp
    src/private/keyvault_protocol.hpp
    src/private/package_version.hpp
    src/recover_deleted_key_operation.cpp
)

add_library(azure-security-keyvault-keys
  ${AZURE_KEYVAULT_KEYS_HEADER} ${AZURE_KEYVAULT_KEYS_SOURCE}
)
create_per_service_target_build(keyvault azure-security-keyvault-keys)
add_library(Azure::azure-security-keyvault-keys ALIAS azure-security-keyvault-keys)

target_include_directories(
  azure-security-keyvault-keys
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

# Include shared source code
# NOTE: Use shared-code only within .cpp files. DO NEVER consume the shared-code from header files.
target_include_directories(
  azure-security-keyvault-keys
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../azure-security-keyvault-shared/inc>
)

target_link_libraries(azure-security-keyvault-keys PUBLIC Azure::azure-core)

target_compile_definitions(azure-security-keyvault-keys PRIVATE _azure_BUILDING_SDK)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-keys azure-security-keyvault-keys-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
set_target_properties(azure-security-keyvault-keys PROPERTIES VERSION ${AZ_LIBRARY_VERSION})
generate_documentation(azure-security-keyvault-keys ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)

  if (NOT AZ_ALL_LIBRARIES OR FETCH_SOURCE_DEPS)
    include(AddGoogleTest)
    enable_testing ()
  endif()
  
  add_subdirectory(test/ut)
  add_subdirectory(test/ut-hsm)
  add_subdirectory(test/macros-build)
endif()

if (BUILD_PERFORMANCE_TESTS)
  add_subdirectory(test/perf)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()

az_vcpkg_export(
    azure-security-keyvault-keys
    SECURITY_KEYVAULT_KEYS
    "azure/keyvault/keys/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-security-keyvault-keys
  SECURITY_KEYVAULT_KEYS
  "azure/keyvault/keys/rtti.hpp"
)

unset(FETCH_SOURCE_DEPS CACHE)
