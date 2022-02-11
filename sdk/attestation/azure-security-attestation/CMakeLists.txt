# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
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
project(azure-security-attestation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)


if(NOT AZ_ALL_LIBRARIES)
  find_package(azure-core-cpp "1.2.0" CONFIG QUIET)
  if(NOT azure-core-cpp_FOUND)
    find_package(azure-core-cpp "1.2.0" REQUIRED)
  endif()
endif()

#if(WIN32)
#    find_package(WIL CONFIG REQUIRED)
#    target_link_libraries(WIL::WIL)
#    target_link_libraries(azure-core PRIVATE bcrypt crypt32)
#else()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-core PUBLIC OpenSSL::Crypto OpenSSL::SSL)
#endif()


set(
  AZURE_ATTESTATION_HEADER
    inc/azure/attestation/dll_import_export.hpp
    inc/azure/attestation/attestation_client.hpp
    inc/azure/attestation.hpp
    inc/azure/attestation/attestation_client_models.hpp
    inc/azure/attestation/attestation_client_options.hpp
    
    src/private/package_version.hpp
    src/private/attestation_common_request.hpp
    src/private/attestation_deserializers_private.hpp
     
    src/private/attestation_client_models_private.hpp
    src/private/crypto/inc/crypto.hpp
    
    src/private/attestation_client_private.hpp)

set(
  AZURE_ATTESTATION_SOURCE
    src/attestation_client.cpp
    src/attestation_client_options.cpp
    src/attestation_common_request.cpp
    src/private/attestation_deserializers_private.cpp
    src/private/jsonhelpers.cpp
    src/private/attestation_client_private.cpp)

#if(WIN32)
#set(
#    AZURE_ATTESTATION_SOURCE
#    ${AZURE_ATTESTATION_SOURCE}
#    src/private/crypto/bcrypt/bcryptcrypto.cpp
#    src/private/crypto/bcrypt/bcryptkeys.cpp
#    src/private/crypto/bcrypt/bcryptcert.cpp)
#else()
set(
    AZURE_ATTESTATION_SOURCE
    ${AZURE_ATTESTATION_SOURCE}
    src/private/crypto/openssl/opensslcrypto.cpp
    src/private/crypto/openssl/opensslkeys.cpp
    src/private/crypto/openssl/opensslcert.cpp)
#endif()

create_per_service_target_build(azure-security-attestation attestation)

add_library(azure-security-attestation
    ${AZURE_ATTESTATION_HEADER}
    ${AZURE_ATTESTATION_SOURCE})

target_include_directories(
  azure-security-attestation
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
      # Disable dependency on azure-core until azure-core has released to vcpkg
      # ${azure-core-cpp_INCLUDE_DIRS}
    PRIVATE
      "src/private"
)

# Disable dependency on azure-core until azure-core has released to vcpkg
target_link_libraries(azure-security-attestation PUBLIC Azure::azure-core)

# make sure that users can consume the project as a library.
add_library(Azure::azure-security-attestation ALIAS azure-security-attestation)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(attestation azure-security-attestation azure-security-attestation-test "tests?/*;samples?/*")

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/src/private/package_version.hpp")
generate_documentation(azure-security-attestation ${AZ_LIBRARY_VERSION})

az_vcpkg_export(
    azure-security-attestation
    ATTESTATION
    "azure/attestation/dll_import_export.hpp"
  )

az_rtti_setup(
  azure-security-attestation
  ATTESTATION
  "azure/attestation/rtti.hpp"
)

if(BUILD_TESTING)
    # tests
    add_subdirectory(test/ut)
endif()
