# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)
project(azure-security-keyvault-keys LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../../cmake-modules")

include(az_version)
include(CodeCoverage)
include(DefineTransportAdapter)
include(doxygen_common)
include(global_compile_options)

add_library(azure-security-keyvault-keys
    src/key_client.cpp
    src/key_type.cpp
    src/key_vault_key.cpp
)

target_include_directories (azure-security-keyvault-keys PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/azure-security-keyvault-keys>)
target_link_libraries(azure-security-keyvault-keys PUBLIC azure-security-keyvault-common)

add_library(azure::keyvault::keys ALIAS azure-security-keyvault-keys)

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-keys azure-security-keyvault-keys-test)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/keyvault/keys/version.hpp")
generate_documentation(azure-security-keyvault-keys ${AZ_LIBRARY_VERSION})

if(BUILD_TESTING)
  add_subdirectory(test)
endif()

if(BUILD_SAMPLES)
  add_subdirectory(sample)
endif()
