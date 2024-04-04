# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")
include(AzureVcpkg)
az_vcpkg_integrate()

cmake_minimum_required (VERSION 3.13)
project(azure-data-tables LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

option(FETCH_SOURCE_DEPS "build source dependencies" OFF)

include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)
include(AzureBuildTargetForCI)
include(CreateMapFile)

if(FETCH_SOURCE_DEPS)
    set(AZ_ALL_LIBRARIES ON)
    include(FolderList)
    SetCompileOptions(DATA_TABLES)
    GetFolderList(DATA_TABLES)
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

set(
  AZURE_DATA_TABLES_HEADER
    inc/azure/data/tables/account_sas_builder.hpp
    inc/azure/data/tables/credentials/azure_sas_credential.hpp
    inc/azure/data/tables/credentials/named_key_credential.hpp
    inc/azure/data/tables/dll_import_export.hpp
    inc/azure/data/tables/enum_operators.hpp
    inc/azure/data/tables/internal/cryptography/hmacsha256.hpp
    inc/azure/data/tables/internal/cryptography/url_encode.hpp
    inc/azure/data/tables/internal/policies/service_version_policy.hpp
    inc/azure/data/tables/internal/policies/shared_key_lite_policy.hpp
    inc/azure/data/tables/internal/policies/tenant_bearer_token_policy.hpp
    inc/azure/data/tables/internal/policies/timeout_policy.hpp
    inc/azure/data/tables/internal/serializers.hpp
    inc/azure/data/tables/internal/xml_wrapper.hpp
    inc/azure/data/tables/models.hpp
    inc/azure/data/tables/rtti.hpp
    inc/azure/data/tables.hpp
    inc/azure/data/tables/tables_clients.hpp
    inc/azure/data/tables/tables_sas_builder.hpp
)

set(
  AZURE_DATA_TABLES_SOURCE
    src/account_sas_builder.cpp
    src/credentials/named_key_credential.cpp
    src/cryptography/hmacsha256.cpp
    src/models.cpp
    src/policies/shared_key_lite_policy.cpp
    src/policies/tenant_bearer_token_policy.cpp
    src/policies/timeout_policy.cpp
    src/private/package_version.hpp
    src/serializers.cpp
    src/tables_clients.cpp
    src/tables_sas_builder.cpp
    src/xml_wrapper.cpp
)

add_library(azure-data-tables ${AZURE_DATA_TABLES_HEADER} ${AZURE_DATA_TABLES_SOURCE})
create_per_service_target_build(tables azure-data-tables)
# make sure that users can consume the project as a library.
add_library(Azure::azure-data-tables ALIAS azure-data-tables)

if(WIN32)
    target_link_libraries(azure-data-tables PRIVATE bcrypt webservices crypt32)
else()
    # Required for Hashing (sha ).
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-data-tables PRIVATE OpenSSL::SSL)
    find_package(LibXml2 REQUIRED)
    target_include_directories(azure-data-tables SYSTEM PRIVATE ${LIBXML2_INCLUDE_DIRS})
    target_link_libraries(azure-data-tables PRIVATE ${LIBXML2_LIBRARIES})
endif()

target_include_directories(
  azure-data-tables
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-data-tables PUBLIC Azure::azure-core)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-data-tables ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-data-tables
    DATA_TABLES
    azure/data/tables/dll_import_export.hpp
  )
  
az_rtti_setup(
  azure-data-tables
  DATA_TABLES
  azure/data/tables/rtti.hpp
)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(tables azure-data-tables azure-data-tables-test "tests?/*;samples?/*")

if(BUILD_TESTING)
  add_compile_definitions(_azure_TABLES_TESTING_BUILD)
  add_subdirectory(test/ut)
  add_subdirectory(test/stress)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(samples)
endif()
