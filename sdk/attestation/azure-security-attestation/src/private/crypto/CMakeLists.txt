# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-security-attestation-crypto LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(AzureVcpkg)
include(AzureVersion)
include(AzureCodeCoverage)
include(AzureTransportAdapters)
include(AzureDoxygen)
include(AzureGlobalCompileOptions)
include(AzureConfigRTTI)

az_vcpkg_integrate()

set(
  CRYPTO_HEADER
    inc/crypto.hpp
    inc/cryptocert.hpp
    inc/cryptokeys.hpp
 )

set(
  CRYPTO_SOURCE
    openssl/opensslkeys.cpp
)

add_library(azure-security-attestation-crypto ${CRYPTO_HEADER} ${CRYPTO_SOURCE})

#if(WIN32)
#    target_link_libraries(azure-core PRIVATE bcrypt crypt32)
#else()
    # Required for Hashing ( md5 and sha ).
    find_package(OpenSSL REQUIRED)
    target_link_libraries(azure-core PUBLIC OpenSSL::Crypto)
#endif()

target_include_directories(
  azure-security-attestation-crypto
    PRIVATE
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>
      $<INSTALL_INTERFACE:include>
)

target_link_libraries(azure-security-attestation-crypto PUBLIC Azure::azure-core)

# make sure that users can consume the project as a library.
#add_library(Azure::azure-security-attestation-crypto ALIAS azure-security-attestation-crypto)
