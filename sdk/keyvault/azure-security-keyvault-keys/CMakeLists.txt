# Copyright (c) Microsoft Corporation. All rights reserved.
# SPDX-License-Identifier: MIT

cmake_minimum_required (VERSION 3.13)

# CodeCoverage will validate requirements
include(CodeCoverage)

add_library(azure-security-keyvault-keys
    src/)
target_include_directories (azure-security-keyvault-keys PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc> $<INSTALL_INTERFACE:include/azure-security-keyvault-keys>)
target_link_libraries(azure-security-keyvault-keys azure-core)
add_library(azure::keyvault::keys ALIAS azure-security-keyvault-keys)

get_az_version("${CMAKE_CURRENT_SOURCE_DIR}/inc/azure/keyvault/keys/version.hpp")

# coverage. Has no effect if BUILD_CODE_COVERAGE is OFF
create_code_coverage(keyvault azure-security-keyvault-keys azure-keyvault-test)

generate_documentation(azure-security-keyvault-keys ${AZ_LIBRARY_VERSION})
